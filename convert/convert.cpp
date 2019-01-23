// convert.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <fbxsdk.h>
#include "Common.h"
#include "GeometryUtility.h"
#include "DataChunk.h"
#include "MemoryStream.h"
#include "MzHeader.h"
#include <vector>
#include <map>
#include "d3d9/d3d9.h"
#include "d3d9/d3dx9tex.h"
#include <fstream>

using namespace std;

static vector<_VertexData> s_aVertexData;
static vector<MODELFACE> s_aFace;
static vector<SUBMESH_V1_3> s_aSubMesh;
static vector<MATERIAL> s_aMaterials;
static vector<BoneData> s_aBoneData;
static vector<MODELANIMATION> s_aAnimations;
static FbxManager* s_pSdkManager = NULL;
static FbxScene* s_pScene = NULL;
static FbxString s_strFilePath;

typedef std::map<uint, float> VERTEX_WEIGHT;
typedef std::map<uint, VERTEX_WEIGHT > BONE_WEIGHT;

static void Log(const char* src, ...)
{
	char buf[1024] = {0};
	va_list argList;
	va_start(argList, src);
	vsnprintf(buf, sizeof(buf)-1, src, argList);
	va_end(argList);
	printf(buf);
}

static string GetPostfixName(const string& sz)
{
	int off = (int)sz.find_last_of(".");
	if (off == -1)
		return "";
	else
		return sz.substr(off+1);
}

static string GetPrefixName(const string& sz)
{
    int off = (int)sz.find_last_of(".");
	if (off == -1)
		return sz;
	else
		return sz.substr(0,off);
}

static string ChangePostfixName(const string& filename, const string& postfixname)
{
	string newlf = postfixname;
	string fn = filename;
	string lf = GetPostfixName(filename);
	string ff = GetPrefixName(filename);
	if (lf == newlf)
		return filename;
	else
		return ff + (newlf==""?"":(string)"."+newlf);
}

// Create a skeleton with 2 segments.
static FbxNode* CreateSkeleton(const char* pName)
{
	// Create skeleton root. 
	FbxString lRootName(pName);
	lRootName += "Root";
	FbxSkeleton* lSkeletonRootAttribute = FbxSkeleton::Create(s_pScene, pName);
	lSkeletonRootAttribute->SetSkeletonType(FbxSkeleton::eRoot);
	FbxNode* lSkeletonRoot = FbxNode::Create(s_pScene,lRootName.Buffer());
	lSkeletonRoot->SetNodeAttribute(lSkeletonRootAttribute);    
	lSkeletonRoot->LclTranslation.Set(FbxVector4(0.0, -40.0, 0.0));

	// Create skeleton first limb node. 
	FbxString lLimbNodeName1(pName);
	lLimbNodeName1 += "LimbNode1";
	FbxSkeleton* lSkeletonLimbNodeAttribute1 = FbxSkeleton::Create(s_pScene,lLimbNodeName1);
	lSkeletonLimbNodeAttribute1->SetSkeletonType(FbxSkeleton::eLimbNode);
	lSkeletonLimbNodeAttribute1->Size.Set(1.0);
	FbxNode* lSkeletonLimbNode1 = FbxNode::Create(s_pScene,lLimbNodeName1.Buffer());
	lSkeletonLimbNode1->SetNodeAttribute(lSkeletonLimbNodeAttribute1);    
	lSkeletonLimbNode1->LclTranslation.Set(FbxVector4(0.0, 40.0, 0.0));
	lSkeletonLimbNode1->LclRotation.Set(FbxVector4(0, 0.0, 80.0));

	// Create skeleton second limb node. 
	FbxString lLimbNodeName2(pName);
	lLimbNodeName2 += "LimbNode2";
	FbxSkeleton* lSkeletonLimbNodeAttribute2 = FbxSkeleton::Create(s_pScene,lLimbNodeName2);
	lSkeletonLimbNodeAttribute2->SetSkeletonType(FbxSkeleton::eLimbNode);
	lSkeletonLimbNodeAttribute2->Size.Set(1.0);
	FbxNode* lSkeletonLimbNode2 = FbxNode::Create(s_pScene,lLimbNodeName2.Buffer());
	lSkeletonLimbNode2->SetNodeAttribute(lSkeletonLimbNodeAttribute2);    
	lSkeletonLimbNode2->LclTranslation.Set(FbxVector4(0.0, 40.0, 0.0));

	// Build skeleton node hierarchy. 
	lSkeletonRoot->AddChild(lSkeletonLimbNode1);
	lSkeletonLimbNode1->AddChild(lSkeletonLimbNode2);

	return lSkeletonRoot;
}

static void SkinSkeleton(FbxSkin* pSkin, FbxNode* pNode, const BONE_WEIGHT& bw)
{
	uint boneid = (uint)pNode->GetUserDataPtr();
	BONE_WEIGHT::const_iterator it = bw.find(boneid);
	if (it != bw.end()) {
		const VERTEX_WEIGHT& vw = it->second;
		if (vw.size() > 0) {
			FbxCluster* pCluster = FbxCluster::Create(s_pScene,"");
			pCluster->SetLink(pNode);
			pCluster->SetLinkMode(FbxCluster::eTotalOne);
			pSkin->AddCluster(pCluster);
			for (VERTEX_WEIGHT::const_iterator it1 = vw.begin(); it1 != vw.end(); it1++) {
				uint vidx = it1->first;
				float weight = it1->second;
				pCluster->AddControlPointIndex(vidx, weight);
			}
			FbxAMatrix mat = pNode->EvaluateGlobalTransform();
			pCluster->SetTransformLinkMatrix(mat);
		}
	}
	for (int i = 0; i < pNode->GetChildCount(); i++) {
		FbxNode* pNode1 = pNode->GetChild(i);
		SkinSkeleton(pSkin, pNode1, bw);
	}
}

void ImportMz()
{
	vector<uchar> aBuf;
	std::ifstream ifs;
	ifs.open(s_strFilePath.Buffer(), std::ios_base::in | std::ios_base::binary);
	if (ifs.is_open()) {
		std::ifstream::pos_type old = ifs.tellg();
		ifs.seekg(0, std::ios_base::end);
		uint len = (uint)ifs.tellg();
		ifs.seekg(old, std::ios_base::beg);
		aBuf.resize(len);
		ifs.read((char*)&aBuf.front(), len);
		ifs.close();
	}

	MemoryStream ms(&aBuf.front(), aBuf.size());
	MemoryStream* pStream = &ms;
	uint ver = 0;
	DataChunk chunk;
	DataChunk::stChunk* pChunk = chunk.beginChunk(pStream);

	s_aVertexData.clear();
	s_aFace.clear();
	s_aMaterials.clear();
	s_aBoneData.clear();
	s_aSubMesh.clear();

	while (pChunk)
	{
		if (pChunk->m_ui32DataSize == 0)
		{
			pChunk = chunk.nextChunk(pStream);
			continue;
		}
		MemoryStream stream((uchar*)pChunk->m_pData,pChunk->m_ui32DataSize);
		switch(pChunk->m_ui32Type)
		{
		case 'MVER':
			{
				stream.read(&ver,sizeof(ver));
			}
			break;
		case 'MVTX':
			{
				uint nVerticeNum = stream.getLength() / sizeof(_VertexData);
				s_aVertexData.resize(nVerticeNum);
				memcpy((void*)&s_aVertexData.front(), stream.getBuffer(), s_aVertexData.size() * sizeof(_VertexData));
			}
			break;
		case 'MFAC':
			{
				uint nFace = stream.getLength() / sizeof(MODELFACE);
				s_aFace.resize(nFace);
				memcpy((void*)&s_aFace.front(), stream.getBuffer(), s_aFace.size() * sizeof(MODELFACE));
			}
			break;
		case 'MSUB':
			{
				uint nSubMesh = stream.getLength() / sizeof(SUBMESH_V1_3);
				if (nSubMesh)
				{
					SUBMESH_V1_3* pSubMesh = (SUBMESH_V1_3*)stream.getBuffer();
					for (uint i = 0; i < nSubMesh; i++)
					{
						s_aSubMesh.push_back(pSubMesh[i]);
					}
				}
			}
			break;
		case 'MMTX':
			{
				uint nMaterials = 0;
				stream.read(&nMaterials, sizeof(nMaterials));
				for (uint i = 0; i < nMaterials; i++)
				{
					MATERIAL mat;

					uchar matNameLen;
					stream.read(&matNameLen, sizeof(matNameLen));
					char matName[128] = {0};
					stream.read(matName, matNameLen);
					mat.name = matName;

					stream.read(&mat.lighting, sizeof(mat.lighting));
					stream.read(&mat.ambient, sizeof(mat.ambient));
					stream.read(&mat.diffuse, sizeof(mat.diffuse));
					stream.read(&mat.specular, sizeof(mat.specular));
					stream.read(&mat.emissive, sizeof(mat.emissive));
					stream.read(&mat.isTransparent, sizeof(mat.isTransparent));
					stream.read(&mat.twoSide, sizeof(mat.twoSide));

					uint nTexture = 0;
					stream.read(&nTexture, sizeof(nTexture));
					for (uint j = 0; j < nTexture; j++)
					{
						TEXTURE tex;

						uchar filenameLen;
						stream.read(&filenameLen, sizeof(filenameLen));
						char str[128] = {0};
						stream.read(str, filenameLen);
						str[filenameLen] = 0;
						tex.name = str;

						stream.read(&tex.opType, sizeof(tex.opType));

						mat.textures.push_back(tex);
					}
					s_aMaterials.push_back(mat);
				}
			}
			break;
		case 'MANM':
			{
				s_aAnimations.clear();

				uint32 nAnims; 
				stream.read(&nAnims,sizeof(nAnims));

				for (uint i = 0; i < nAnims; i++)
				{
					MODELANIMATION ani;

					uchar AnimnameLen; 
					stream.read(&AnimnameLen,sizeof(AnimnameLen));
					char str[64];
					stream.read(str,AnimnameLen);
					str[AnimnameLen] = 0;
					ani.name = str;

					uint startTime,endTime;
					stream.read(&startTime,sizeof(startTime));
					stream.read(&endTime,sizeof(endTime));
					ani.startTime = startTime;
					ani.endTime = endTime;

					ani.loopType = 1;

					s_aAnimations.push_back(ani);

				}
			}
			break;
		case 'MBON':
			{
				uint nBones = 0;
				stream.read(&nBones,sizeof(nBones));
				for (uint i = 0; i < nBones; i++)
				{
					BoneData bone;

					int id;
					stream.read(&id, sizeof(id));

					uchar JointnameLen; 
					stream.read(&JointnameLen, sizeof(JointnameLen));
					bone.name.resize(JointnameLen+1);
					stream.read((char*)bone.name.data(), JointnameLen);

					int parent;
					stream.read(&parent,sizeof(parent));
					Vector3 pivot;
					stream.read(&pivot,sizeof(pivot));

					//读取模型的初始变换
					float _t[3],_r[4],_s[3];
					stream.read(_t, sizeof(float)*3);
					stream.read(_r, sizeof(float)*4);
					stream.read(_s, sizeof(float)*3);
					bone.initTrans = Vector3(_t[0],_t[1],_t[2]);
					bone.initQuat = Quaternion(_r[0],_r[1],_r[2],_r[3]);
					bone.initScale = Vector3(_s[0],_s[1],_s[2]);

					bone.billboarded = false;
					bone.billboardedLockX = false;
					bone.billboardedLockY =false;
					bone.billboardedLockZ = false;
					bone.cameraAnchored = false;
					bone.dontInheritRotation = false;
					bone.dontInheritScaling = false;
					bone.dontInheritTranslation = false;

					bone.objectId = id;
					bone.parentId = parent;
					bone.pivotPoint = pivot;
					bone.pivotRotation = false;

					////开启插值运算 mod by gxj 2013-8-14
					////pData->translation.setInterpolationType(INTERPOLATION_NONE);
					////pData->rotation.setInterpolationType(INTERPOLATION_NONE);
					////pData->scale.setInterpolationType(INTERPOLATION_NONE);
					//pData->translation.setInterpolationType(INTERPOLATION_LINEAR);
					//pData->rotation.setInterpolationType(INTERPOLATION_LINEAR);
					//pData->scale.setInterpolationType(INTERPOLATION_LINEAR);

					Log("Bones id=%d name=%s\n", i, bone.name.c_str());

					uint nKeyframes = 0;
					stream.read(&nKeyframes, sizeof(nKeyframes));
					for (uint j = 0; j < nKeyframes; j++)
					{
						ModelKeyframeTranslation kf;
						stream.read(&kf, sizeof(kf));
						bone.keyframesTranslation.push_back(kf);
					}
					nKeyframes = 0;
					stream.read(&nKeyframes, sizeof(nKeyframes));
					for (uint j = 0; j < nKeyframes; j++)
					{
						ModelKeyframeRotation kf;
						stream.read(&kf, sizeof(kf));
						bone.keyframesRotation.push_back(kf);
					}
					nKeyframes = 0;
					stream.read(&nKeyframes, sizeof(nKeyframes));
					for (uint j = 0; j < nKeyframes; j++)
					{
						ModelKeyframeScale kf;
						stream.read(&kf, sizeof(kf));
						bone.keyframesScale.push_back(kf);
					}

					bool hasRibbonSystem;
					bool hasParticleSystem;
					stream.read(&hasRibbonSystem,sizeof(hasRibbonSystem));
					stream.read(&hasParticleSystem,sizeof(hasParticleSystem));
					if (hasRibbonSystem)
					{
						bool visible;
						stream.read(&visible,sizeof(visible));
						float above;
						stream.read(&above,sizeof(above));
						float below;
						stream.read(&below,sizeof(below));
						short edgePerSecond;
						stream.read(&edgePerSecond,sizeof(edgePerSecond));
						float edgeLife;
						stream.read(&edgeLife,sizeof(edgeLife));
						float gravity;
						stream.read(&gravity,sizeof(gravity));
						short rows;
						stream.read(&rows,sizeof(rows));
						short cols;
						stream.read(&cols,sizeof(cols));
						short slot;
						stream.read(&slot,sizeof(slot));
						Color3 color;
						stream.read(&color,sizeof(color));
						float alpha;
						stream.read(&alpha,sizeof(alpha));
						short blendMode;
						stream.read(&blendMode,sizeof(blendMode));
						uchar textureFilenameLen;
						stream.read(&textureFilenameLen,sizeof(textureFilenameLen));
						char filename[256];
						stream.read(filename,textureFilenameLen);
						filename[textureFilenameLen] = 0;

					}
					if (hasParticleSystem)
					{
						bool visible;
						stream.read(&visible,sizeof(visible));
						float speed;
						stream.read(&speed,sizeof(speed));
						float variation;
						stream.read(&variation,sizeof(variation));
						float coneAngle;
						stream.read(&coneAngle,sizeof(coneAngle));
						float gravity;
						stream.read(&gravity,sizeof(gravity));
						float life;
						stream.read(&life,sizeof(life));
						float emissionRate;
						stream.read(&emissionRate,sizeof(emissionRate));
						float width;
						stream.read(&width,sizeof(width));
						float length;
						stream.read(&length,sizeof(length));
						short blendMode;
						stream.read(&blendMode,sizeof(blendMode));
						short rows;
						stream.read(&rows,sizeof(rows));
						short cols;
						stream.read(&cols,sizeof(cols));
						float tailLength;
						stream.read(&tailLength,sizeof(tailLength));
						float timeMiddle;
						stream.read(&timeMiddle,sizeof(timeMiddle));
						Color3 colorStart;
						stream.read(&colorStart,sizeof(colorStart));
						Color3 colorMiddle;
						stream.read(&colorMiddle,sizeof(colorMiddle));
						Color3 colorEnd;
						stream.read(&colorEnd,sizeof(colorEnd));
						Vector3 alpha;
						stream.read(&alpha,sizeof(alpha));
						Vector3 scale;
						stream.read(&scale,sizeof(scale));
						typedef short short3[3];
						short3 headLifeSpan;
						stream.read(&headLifeSpan,sizeof(headLifeSpan));
						short3 headDecay;
						stream.read(&headDecay,sizeof(headDecay));
						short3 tailLifeSpan;
						stream.read(&tailLifeSpan,sizeof(tailLifeSpan));
						short3 tailDecay;
						stream.read(&tailDecay,sizeof(tailDecay));
						bool head,tail,unshaded,unfogged;
						stream.read(&head,sizeof(head));
						stream.read(&tail,sizeof(tail));
						stream.read(&unshaded,sizeof(unshaded));
						stream.read(&unfogged,sizeof(unfogged));
						uchar textureFilenameLen;
						stream.read(&textureFilenameLen,sizeof(textureFilenameLen));
						char filename[256];
						stream.read(filename,textureFilenameLen);
						filename[textureFilenameLen] = 0;

					}

					s_aBoneData.push_back(bone);
				}
			}
		}
		pChunk = chunk.nextChunk(pStream);
	}

}

void ExportFbx_Ani(std::map<int, FbxNode*>& mapNodeBones)
{
	//动画
	for (vector<MODELANIMATION>::iterator it = s_aAnimations.begin(); it != s_aAnimations.end(); it++) {
		const MODELANIMATION& ma = *it;
		FbxTime lTime;
		int lKeyIndex = 0;
		FbxAnimStack* pAnimStack = FbxAnimStack::Create(s_pScene, ma.name.c_str());
		FbxAnimLayer* pAnimLayer = FbxAnimLayer::Create(s_pScene, "Base Layer");
		pAnimStack->AddMember(pAnimLayer);
		for (uint i = 0; i < s_aBoneData.size(); i++) {
			const BoneData& bone = s_aBoneData[i];
			auto it = mapNodeBones.find(i);
			if (it != mapNodeBones.end()) {
				FbxNode* pBone = it->second;

				//平移
				FbxAnimCurve* pCurveTransX = pBone->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
				FbxAnimCurve* pCurveTransY = pBone->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
				FbxAnimCurve* pCurveTransZ = pBone->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
				if (pCurveTransX && pCurveTransY && pCurveTransZ) {
					pCurveTransX->KeyModifyBegin();
					pCurveTransY->KeyModifyBegin();
					pCurveTransZ->KeyModifyBegin();
					for (size_t j = 0; j < bone.keyframesTranslation.size(); j++) {
						const ModelKeyframeTranslation& keyfrm = bone.keyframesTranslation[j];
						lTime.SetSecondDouble(keyfrm.time/1000.f);
						lKeyIndex = pCurveTransX->KeyAdd(lTime);
						pCurveTransX->KeySetValue(lKeyIndex, keyfrm.v[0]);
						pCurveTransX->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
						lKeyIndex = pCurveTransY->KeyAdd(lTime);
						pCurveTransY->KeySetValue(lKeyIndex, keyfrm.v[1]);
						pCurveTransY->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
						lKeyIndex = pCurveTransZ->KeyAdd(lTime);
						pCurveTransZ->KeySetValue(lKeyIndex, keyfrm.v[2]);
						pCurveTransZ->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
					}
					pCurveTransX->KeyModifyEnd();
					pCurveTransY->KeyModifyEnd();
					pCurveTransZ->KeyModifyEnd();
				}

				//旋转
				FbxAnimCurve* pCurveRotX = pBone->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
				FbxAnimCurve* pCurveRotY = pBone->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
				FbxAnimCurve* pCurveRotZ = pBone->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
				if (pCurveRotX && pCurveRotY && pCurveRotZ) {
					pCurveRotX->KeyModifyBegin();
					pCurveRotY->KeyModifyBegin();
					pCurveRotZ->KeyModifyBegin();
					for (size_t j = 0; j < bone.keyframesRotation.size(); j++) {
						const ModelKeyframeRotation& keyfrm = bone.keyframesRotation[j];
						lTime.SetSecondDouble(keyfrm.time/1000.f);
						FbxQuaternion fq(keyfrm.q[0], keyfrm.q[1], keyfrm.q[2], keyfrm.q[3]);
						FbxVector4 rot; rot.SetXYZ(fq);
						lKeyIndex = pCurveRotX->KeyAdd(lTime);
						pCurveRotX->KeySetValue(lKeyIndex, rot[0]);
						pCurveRotX->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
						lKeyIndex = pCurveRotY->KeyAdd(lTime);
						pCurveRotY->KeySetValue(lKeyIndex, rot[1]);
						pCurveRotY->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
						lKeyIndex = pCurveRotZ->KeyAdd(lTime);
						pCurveRotZ->KeySetValue(lKeyIndex, rot[2]);
						pCurveRotZ->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
					}
					pCurveRotX->KeyModifyEnd();
					pCurveRotY->KeyModifyEnd();
					pCurveRotZ->KeyModifyEnd();
				}

				//缩放
				FbxAnimCurve* pCurveSclX = pBone->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
				FbxAnimCurve* pCurveSclY = pBone->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
				FbxAnimCurve* pCurveSclZ = pBone->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
				if (pCurveSclX && pCurveSclY && pCurveSclZ) {
					pCurveSclX->KeyModifyBegin();
					pCurveSclY->KeyModifyBegin();
					pCurveSclZ->KeyModifyBegin();
					for (size_t j = 0; j < bone.keyframesScale.size(); j++) {
						const ModelKeyframeScale& keyfrm = bone.keyframesScale[j];
						lTime.SetSecondDouble(keyfrm.time/1000.f);
						lKeyIndex = pCurveSclX->KeyAdd(lTime);
						pCurveSclX->KeySetValue(lKeyIndex, keyfrm.v[0]);
						pCurveSclX->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
						lKeyIndex = pCurveSclY->KeyAdd(lTime);
						pCurveSclY->KeySetValue(lKeyIndex, keyfrm.v[1]);
						pCurveSclY->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
						lKeyIndex = pCurveSclZ->KeyAdd(lTime);
						pCurveSclZ->KeySetValue(lKeyIndex, keyfrm.v[2]);
						pCurveSclZ->KeySetInterpolation(lKeyIndex, FbxAnimCurveDef::eInterpolationCubic);
					}
					pCurveSclX->KeyModifyEnd();
					pCurveSclY->KeyModifyEnd();
					pCurveSclZ->KeyModifyEnd();
				}

			}
		}

	}
}


void ExportFbx_Mesh()
{

	//模型

	//uint numSubMesh = s_aSubMesh.size();
	uint numSubMesh = 1;

	for (uint idx = 0; idx < numSubMesh; idx++) {

		const SUBMESH_V1_3& ms = s_aSubMesh[idx];

		s_pSdkManager = nullptr;
		s_pScene = nullptr;
		InitializeSdkObjects(s_pSdkManager, s_pScene);

		FbxNode* pNode = FbxNode::Create(s_pScene, ms.name);
		s_pScene->GetRootNode()->AddChild(pNode);
		FbxMesh* pMesh = FbxMesh::Create(s_pScene, ms.name);
		pNode->SetNodeAttribute(pMesh);

		//顶点
		pMesh->InitControlPoints(ms.vcount);
		for (uint i = 0; i < ms.vcount; i++) {
			const _VertexData& vd = s_aVertexData[ms.vstart + i];
			FbxVector4 vec(vd.pos[0], vd.pos[1], vd.pos[2]);
			pMesh->SetControlPointAt(vec, i);
		}

		//索引
		uint face_start = ms.istart/3;
		uint face_num = ms.icount/3;
		for (uint i = 0; i < face_num; i++) {
			const MODELFACE& face = s_aFace[face_start + i];
			pMesh->BeginPolygon();
			pMesh->AddPolygon(face.index[0]-ms.vstart);
			pMesh->AddPolygon(face.index[1]-ms.vstart);
			pMesh->AddPolygon(face.index[2]-ms.vstart);
			pMesh->EndPolygon();
		}

		//法线
		FbxGeometryElementNormal* pNormal = pMesh->CreateElementNormal();
		pNormal->SetMappingMode(FbxGeometryElement::eByControlPoint);
		pNormal->SetReferenceMode(FbxGeometryElement::eDirect);
		for (uint i = 0; i < ms.vcount; i++) {
			const _VertexData& vd = s_aVertexData[ms.vstart + i];
			FbxVector4 vec(vd.normal[0], vd.normal[1], vd.normal[2]);
			pNormal->GetDirectArray().Add(vec);
		}

		//UV
		FbxGeometryElementUV* pUV1 = pMesh->CreateElementUV("UVSet1");
		pUV1->SetMappingMode(FbxGeometryElement::eByControlPoint);
		pUV1->SetReferenceMode(FbxGeometryElement::eDirect);
		for (uint i = 0; i < ms.vcount; i++) {
			const _VertexData& vd = s_aVertexData[ms.vstart + i];
			//翻转V
			FbxVector2 vec(vd.texcoords[0], 1.f - vd.texcoords[1]);
			pUV1->GetDirectArray().Add(vec);
		}

		//材质
		if (ms.matId < s_aMaterials.size()) {

			const MATERIAL& mat = s_aMaterials[ms.matId];

			FbxSurfacePhong* pMaterial = FbxSurfacePhong::Create(pMesh->GetScene(), mat.name.c_str());

			//漫反射
			FbxDouble3 diffuse(1, 1, 1);
			pMaterial->Diffuse.Set(diffuse);
			pMaterial->DiffuseFactor = 1;

			//自发光
			FbxDouble3 emissive(0, 0, 0);
			pMaterial->Emissive.Set(emissive);
			pMaterial->EmissiveFactor = 0;

			//环境光
			FbxDouble3 ambient(0, 0, 0);
			pMaterial->Ambient.Set(ambient);
			pMaterial->AmbientFactor = 0;

			//镜面光
			FbxDouble3 specular(0, 0, 0);
			pMaterial->Specular.Set(specular);
			pMaterial->SpecularFactor = 0;

			//是否透明
			pMaterial->TransparencyFactor = mat.isTransparent ? 1 : 0;

			FbxNode* lNode = pMesh->GetNode();
			if (lNode) {
				lNode->AddMaterial(pMaterial);
			}

			//贴图
			if (!mat.textures.empty()) {
				const TEXTURE& tex = mat.textures[0];

				FbxFileTexture* pTexture = FbxFileTexture::Create(pMesh->GetScene(), "Diffuse Texture");
				// Set texture properties.
				//pTexture->SetFileName(tex.name.c_str()); // Resource file is in current directory.
				pTexture->SetFileName(ChangePostfixName(tex.name.c_str(), "png").c_str());
				pTexture->SetTextureUse(FbxTexture::eStandard);
				pTexture->SetMappingType(FbxTexture::eUV);
				pTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
				pTexture->SetSwapUV(false);
				pTexture->SetTranslation(0.0, 0.0);
				pTexture->SetScale(1.0, 1.0);
				pTexture->SetRotation(0.0, 0.0);
				pMaterial->Diffuse.ConnectSrcObject(pTexture);
			}

		}

		//骨骼
		FbxNode* pRootNode = NULL;
		std::map<int, FbxNode*> mapNodeBones;
		if (!s_aBoneData.empty()) {
			for (uint i = 0; i < s_aBoneData.size(); i++) {
				const BoneData& bone = s_aBoneData[i];
				FbxSkeleton* pBoneAttribute = FbxSkeleton::Create(s_pScene, bone.name.c_str());
				if (bone.parentId == -1) {
					pBoneAttribute->SetSkeletonType(FbxSkeleton::eRoot);
				}
				else {
					pBoneAttribute->SetSkeletonType(FbxSkeleton::eLimbNode);
					pBoneAttribute->Size.Set(1.0);
				}
				FbxString str_name = bone.name.c_str();
				FbxNode* pBoneNode = FbxNode::Create(s_pScene, str_name);
				pBoneNode->SetUserDataPtr((void*)bone.objectId);
				pBoneNode->SetNodeAttribute(pBoneAttribute);
				Vector3 pos = bone.initTrans;
				if (bone.parentId != -1) {
					pos = pos - s_aBoneData[bone.parentId].initTrans;
				}
				pBoneNode->LclTranslation.Set(FbxVector4(pos.d_x, pos.d_y, pos.d_z));
				//Quaternion q = bone.initQuat;
				//if (bone.parentId != -1) {
				//	q = aBoneData[bone.parentId]->initQuat * q;
				//}
				//FbxQuaternion fq(q.x, q.y, q.z, q.w);
				//FbxVector4 rot;
				//rot.SetXYZ(fq);
				//pBoneNode->LclRotation.Set(rot);
				pBoneNode->LclScaling.Set(FbxVector4(bone.initScale.d_x, bone.initScale.d_y, bone.initScale.d_z));
				mapNodeBones[bone.objectId] = pBoneNode;
				auto it = mapNodeBones.find(bone.parentId);
				if (it != mapNodeBones.end()) {
					it->second->AddChild(pBoneNode);
				}
				else {
					pRootNode = pBoneNode;
					s_pScene->GetRootNode()->AddChild(pBoneNode);
				}
			}
		}
		 
		BONE_WEIGHT bw;
		for (uint i = 0; i < ms.vcount; i++) {
			const _VertexData& vd = s_aVertexData[ms.vstart + i];
			for (int j = 0; j < 4; j++) {
				if (vd.bones[j] < s_aBoneData.size()) {
					VERTEX_WEIGHT& vw = bw[vd.bones[j]];
					vw[i] = vd.weights[j];
				}
			}
		}

		FbxSkin* pSkin = FbxSkin::Create(s_pScene, "");
		pMesh->AddDeformer(pSkin);

		SkinSkeleton(pSkin, pRootNode, bw);

		if (idx == 0) {
			ExportFbx_Ani(mapNodeBones);
		}

		string lFilePath = ChangePostfixName(s_strFilePath.Buffer(), "").c_str();
		lFilePath = lFilePath + "_" + ms.name + ".fbx";

		SaveScene(s_pSdkManager, s_pScene, lFilePath.c_str());

		DestroySdkObjects(s_pSdkManager, true);

	}

}

void ImportScene()
{
	ImportMz();
	ExportFbx_Mesh();
}

int _tmain(int argc, _TCHAR* argv[])
{
	//s_strFilePath = "Res/MONKEYMAN/MONKEYMAN.MZ";
	s_strFilePath = "Res/SWORDSMAN/Swordsman.MZ";
	ImportScene();
	return 0;
}

