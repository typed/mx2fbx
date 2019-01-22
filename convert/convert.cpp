// convert.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <fbxsdk.h>
#include "Common.h"
#include "GeometryUtility.h"
#include "DataChunk.h"
#include "MemoryStream.h"
#include "FileStream.h"
#include "MzHeader.h"
#include <vector>
#include <map>
#include "d3d9/d3d9.h"
#include "d3d9/d3dx9tex.h"
#include <fstream>

using namespace std;

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

void ImportMz(const FbxString& lFilePath, FbxScene* pScene)
{
	uchar* buf = nullptr;
	uint len = 0;
	std::ifstream ifs;
	ifs.open(lFilePath.Buffer(), std::ios_base::in | std::ios_base::binary);
	if (ifs.is_open()) {
		std::ifstream::pos_type old = ifs.tellg();
		ifs.seekg(0, std::ios_base::end);
		len = (uint)ifs.tellg();
		ifs.seekg(old, std::ios_base::beg);
		buf = new uchar[len];
		ifs.read((char*)buf, len);
		ifs.close();
	}

	MemoryStream ms(buf, len);
	MemoryStream* pStream = &ms;
	uint ver = 0;
	DataChunk chunk;
	DataChunk::stChunk* pChunk = chunk.beginChunk(pStream);

	_VertexData* pVertexData = NULL;
	uint nVerticeNum = 0;
	MODELFACE* pFace = NULL;
	uint nFace = 0;

	vector<SUBMESH_V1_3> aSubMesh;
	vector<MATERIAL> aMaterials;

	vector<BoneData> aBoneData;

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
				nVerticeNum = stream.getLength() / sizeof(_VertexData);
				pVertexData = (_VertexData*)stream.getBuffer();
			}
			break;
		case 'MFAC':
			{
				nFace = stream.getLength() / sizeof(MODELFACE);
				pFace = (MODELFACE*)stream.getBuffer();
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
						aSubMesh.push_back(pSubMesh[i]);
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
					aMaterials.push_back(mat);
				}
			}
			break;
		case 'MANM':
			{
				//uint32 nAnims; 
				//stream.read(&nAnims,sizeof(nAnims));
				//MODELANIMATION *pAnims = (MODELANIMATION *)(stream.getBuffer() + stream.getPosition());

				//for(uint i = 0;i < nAnims;i++)
				//{
				//	uchar AnimnameLen; 
				//	stream.read(&AnimnameLen,sizeof(AnimnameLen));
				//	char str[64];
				//	stream.read(str,AnimnameLen);
				//	str[AnimnameLen] = 0;

				//	uint startTime,endTime;
				//	stream.read(&startTime,sizeof(startTime));
				//	stream.read(&endTime,sizeof(endTime));

				//	Animation *pAnimation = new Animation(startTime,endTime,true,str);
				//	//modified by xxh 20091011， 下面那样写不好，会使得m_vAnimations和m_AnimationMap不一致，改为这样。
				//	if( pAnimation)
				//	{
				//		m_vAnimations.push_back(pAnimation);
				//		if( pAnimation && m_AnimationMap.find(pAnimation->getName() ) == m_AnimationMap.end() )
				//			m_AnimationMap[pAnimation->getName() ] = pAnimation;
				//	}
				//}
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

					aBoneData.push_back(bone);
				}
			}
		}
		pChunk = chunk.nextChunk(pStream);
	}

	//模型

	FbxNode* pNodeMesh = FbxNode::Create(pScene, "Mesh");
	pScene->GetRootNode()->AddChild(pNodeMesh);

	for (uint idx = 0; idx < aSubMesh.size(); idx++) {

		const SUBMESH_V1_3& ms = aSubMesh[idx];
		
		FbxNode* pNode = FbxNode::Create(pScene, ms.name);
		pNodeMesh->AddChild(pNode);
		FbxMesh* pMesh = FbxMesh::Create(pScene, ms.name);
		pNode->SetNodeAttribute(pMesh);

		//顶点
		pMesh->InitControlPoints(ms.vcount);
		for (uint i = 0; i < ms.vcount; i++) {
			const _VertexData& vd = pVertexData[ms.vstart + i];
			FbxVector4 vec(vd.pos[0], vd.pos[1], vd.pos[2]);
			pMesh->SetControlPointAt(vec, i);
		}

		//索引
		uint face_start = ms.istart/3;
		uint face_num = ms.icount/3;
		for (uint i = 0; i < face_num; i++) {
			const MODELFACE& face = pFace[face_start + i];
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
			const _VertexData& vd = pVertexData[ms.vstart + i];
			FbxVector4 vec(vd.normal[0], vd.normal[1], vd.normal[2]);
			pNormal->GetDirectArray().Add(vec);
		}

		//UV
		FbxGeometryElementUV* pUV1 = pMesh->CreateElementUV("UVSet1");
		pUV1->SetMappingMode(FbxGeometryElement::eByControlPoint);
		pUV1->SetReferenceMode(FbxGeometryElement::eDirect);
		for (uint i = 0; i < ms.vcount; i++) {
			const _VertexData& vd = pVertexData[ms.vstart + i];
			//翻转V
			FbxVector2 vec(vd.texcoords[0], 1.f - vd.texcoords[1]);
			pUV1->GetDirectArray().Add(vec);
		}

		//材质
		if (ms.matId < aMaterials.size()) {

			const MATERIAL& mat = aMaterials[ms.matId];

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
				pTexture->SetFileName(tex.name.c_str()); // Resource file is in current directory.
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

	}

	//骨骼
	if (!aBoneData.empty()) {
		FbxNode* pNodeSkeleton = FbxNode::Create(pScene, "Skeleton");
		pScene->GetRootNode()->AddChild(pNodeSkeleton);
		FbxSkeleton* lSkeletonRootAttribute = FbxSkeleton::Create(pScene, "SkeletonRoot");
		lSkeletonRootAttribute->SetSkeletonType(FbxSkeleton::eRoot);
		pNodeSkeleton->SetNodeAttribute(lSkeletonRootAttribute);    
		pNodeSkeleton->LclTranslation.Set(FbxVector4(0.0, 0.0, 0.0));
		std::map<int, FbxNode*> mapNodeBones;
		for (auto it = aBoneData.begin(); it != aBoneData.end(); it++) {
			const BoneData& bone = *it;
			FbxSkeleton* pBoneAttribute = FbxSkeleton::Create(pScene, bone.name.c_str());
			pBoneAttribute->SetSkeletonType(FbxSkeleton::eLimbNode);
			FbxNode* pBoneNode = FbxNode::Create(pScene, bone.name.c_str());
			pBoneNode->SetNodeAttribute(pBoneAttribute);
			pBoneNode->LclTranslation.Set(FbxVector4(bone.initTrans.d_x, bone.initTrans.d_y, bone.initTrans.d_z));
			mapNodeBones[bone.objectId] = pBoneNode;
			if (bone.parentId == -1) {
				pNodeSkeleton->AddChild(pBoneNode);
			}
			else {
				auto it1 = mapNodeBones.find(bone.objectId);
				if (it1 != mapNodeBones.end()) {
					it1->second->AddChild(pBoneNode);
				}
			}
		}
	}

	//权重

	//动画

	delete[] buf;
}

void ImportScene(const FbxString& lFilePath)
{
	FbxManager* pSdkManager = NULL;
	FbxScene* pScene = NULL;
	InitializeSdkObjects(pSdkManager, pScene);
	
	ImportMz(lFilePath, pScene);

	SaveScene(pSdkManager, pScene, ChangePostfixName(lFilePath.Buffer(), "fbx").c_str());

	DestroySdkObjects(pSdkManager, true);
}

int _tmain(int argc, _TCHAR* argv[])
{
	//ImportScene("Res/MONKEYMAN/MONKEYMAN.MZ");
	ImportScene("Res/SWORDSMAN/Swordsman.MZ");
	return 0;
}

