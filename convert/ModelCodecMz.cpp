#include "StdAfx.h"
#include "ModelCodecMz.h"
#include "RenderEngine/MzHeader.h"

namespace rkt
{
	template<class T,class OT> void readKeyFrames(Stream *pStream,KeyFrames<T> *pKeyFrames,uint lastTime = 0)
	{
		uint numKeyFrames;
		pStream->read(&numKeyFrames,sizeof(numKeyFrames));
		for(uint i = 0;i < numKeyFrames;i++)
		{
			int time;
			OT data;
			pStream->read(&time,sizeof(time));
			pStream->read(&data,sizeof(data));
			KeyFrame<T> keyFrame;
			keyFrame.time = lastTime + time;
			keyFrame.v = data;
			keyFrame.in = data;
			keyFrame.out = data;
			pKeyFrames->addKeyFrame(keyFrame);
		}
	}

	uint ModelCodecMz::getNumSubModels()
	{
		return m_vRenderPasses.size();
	}

	ISubModel*	ModelCodecMz::getSubModel(const char* name)
	{
		size_t size = m_vRenderPasses.size();
		for(uint i = 0;i < size;i++)
		{
			if(StringHelper::casecmp(m_vRenderPasses[i].m_name,name) == 0)
			{
				return &m_vRenderPasses[i];
			}
		}
		return 0;
	}

	ISubModel*	ModelCodecMz::getSubModel(int index)
	{
		if (index < 0 || index >= (int)m_vRenderPasses.size())
			return 0;
		return &m_vRenderPasses[index];
	}

	Animation* ModelCodecMz::getAnimation(uint index)
	{
		if(index >= m_vAnimations.size())
			return 0;
		return m_vAnimations[index];
	}

	uint	ModelCodecMz::getAnimationCount()
	{
		return m_vAnimations.size();
	}

	Animation* ModelCodecMz::hasAnimation(const std::string& animation)
	{
		std::map<std::string, Animation*>::iterator it = m_AnimationMap.find(animation.c_str());
		if(it == m_AnimationMap.end())
			return 0;
		return it->second;
	}

	Animation* ModelCodecMz::randomAnimation()
	{
		if (m_AnimationMap.empty())
			return 0;
		std::map<std::string, Animation*>::iterator it = m_AnimationMap.begin();
		int num = rand() % m_AnimationMap.size();
		for (int i = 0; i < num; i++, it++);
		return it->second;
	}

	uint	ModelCodecMz::numBones()
	{
		return m_vBones.size();
	}

	BoneData*	ModelCodecMz::getBone(uint index)
	{
		if(index >= m_vBones.size())return 0;
		return m_vBones[index];
	}

	IIndexBuffer*	ModelCodecMz::getIndexBuffer()
	{
		return m_pIB;
	}

	IVertexBuffer*	ModelCodecMz::getTexcoordBuffer()
	{
		return m_pVBTexcoord;
	}

	uint ModelCodecMz::getNumFaces()
	{
		return m_pIB ? m_pIB->getNumIndices() / 3 : 0;
	}

	uint ModelCodecMz::getNumVertices()
	{
		return m_ui32VerticeNum;
	}

	const std::string& ModelCodecMz::getFileName()
	{
		return m_strName;
	}

	uint ModelCodecMz::getVer()
	{
		return m_ver;
	}

	IVertexBuffer* ModelCodecMz::vertexBlend(Skeleton *pSkeleton,const AnimationTime *pTime)
	{
		if (!m_bAnimatedGeometry)
			return m_pVBPosNormal;
		if (!m_ui32VerticeNum || !pTime || !pSkeleton)
			return 0;
		IVertexBuffer*& pVB = m_pVBs;
		if (pVB == NULL) {
			pVB = m_pRenderSystem->getBufferManager()->createVertexBuffer(12, m_ui32VerticeNum << 1, BU_DYNAMIC_WRITE_ONLY, CBF_IMMEDIATE);
			if (pVB == NULL)
				return NULL;
		}
		Vector3* pData = (Vector3*)pVB->lock(0,0,BL_DISCARD);
		if(pData)
		{
			Vector3 *pSkinPos = &pData[0];
			Vector3 *pSkinNormal = &pData[m_ui32VerticeNum];

			for(uint i = 0;i < m_ui32VerticeNum;i++)
			{
				Vector3 v(0,0,0),n(0,0,0);
				_VertexData *ov = m_pVertexData + i;

				for(uint b = 0;b < 4;b++) 
				{
					if(ov->weights[b] > 0&&ov->bones[b]>=0&&ov->bones[b]<pSkeleton->getBoneCount()) 
					{
						Vector3 tv,tn;
						Vector3 otv = Vector3(ov->pos[0],ov->pos[1],ov->pos[2]);

						Bone *pBone = pSkeleton->getBone(ov->bones[b]);
						tv = pBone->getFullTransform() * otv;

						Vector3 otn = Vector3(ov->normal[0],ov->normal[1],ov->normal[2]);
						tn = pBone->getFullRotation() * otn;

						v += tv * ((float)ov->weights[b]);
						n += tn * ((float)ov->weights[b]);
					}
				}
				pSkinPos[i] = v;
				n.normalize();
				pSkinNormal[i] = n;
			}
			pVB->unlock();
		}

		return pVB;
	}

	//根据动画时间来获取定点
	IVertexBuffer* ModelCodecMz::getVertexByAniTime(const AnimationTime *pTime)
	{
		if (!m_bAnimatedGeometry)
			return m_pVBPosNormal;
		if (!m_ui32VerticeNum || !pTime)
			return NULL;
		return m_pVBs;
	}


	uint ModelCodecMz::numParticleSystem()
	{
		return m_vParticleEmitters.size();
	}

	ParticleEmitter2Data*	ModelCodecMz::getParticleSystemData(uint index)
	{
		if(index >= m_vParticleEmitters.size())return 0;
		return m_vParticleEmitters[index];
	}
	
	uint ModelCodecMz::numRibbonSystem()
	{
		return m_vRibbonEmitters.size();
	}

	RibbonEmitterData*	ModelCodecMz::getRibbonSystemData(uint index)
	{
		if(index >= m_vRibbonEmitters.size())return 0;
		return m_vRibbonEmitters[index];
	}

	uint ModelCodecMz::numMmParticleSystem()
	{
		return 0;
	}

	MmParticleSystemData* ModelCodecMz::getMmParticleSystem(uint index)
	{
		return 0;
	}

	const AABB&	ModelCodecMz::getAABB()
	{
		if( !m_bCalculatedAABB)
		{
			if( m_pVertexData)  calcAABB(m_pVertexData,m_ui32VerticeNum,m_AABB,m_BoundingSphere);
			m_bCalculatedAABB =true; 
		}
		return m_AABB;
	}

	const Sphere& ModelCodecMz::getBoudingSphere()
	{
		if( !m_bCalculatedAABB)
		{
			if( m_pVertexData) calcAABB(m_pVertexData,m_ui32VerticeNum,m_AABB,m_BoundingSphere);
			m_bCalculatedAABB = true;
		}
		return m_BoundingSphere;
	}

	const char*	ModelCodecMz::getType() const
	{
		static std::string strType = "mz";
		return strType.c_str();
	}

	void ModelCodecMz::loadFeiBian(rkt::Stream *pStream,const std::string& animationName)
	{
		uint lastTime = 0;
		Animation *pAnimation = 0;
		rkt::DataChunk chunk;
		rkt::DataChunk::stChunk *pChunk = chunk.beginChunk(pStream);
		while(pChunk)
		{
			if(pChunk->m_ui32DataSize == 0)
			{
				pChunk = chunk.nextChunk(pStream);
				continue;
			}
			rkt::MemoryStream stream((uchar*)pChunk->m_pData,pChunk->m_ui32DataSize);
			switch(pChunk->m_ui32Type)
			{
			case 'MVER':
				{
					uint ver;
					stream.read(&ver,sizeof(ver));
					if (ver != m_ver)
						return;
				}
				break;
			case 'MANM':
				{
					uint startTime,endTime;
					stream.read(&startTime, sizeof(startTime));
					stream.read(&endTime, sizeof(endTime));
					if (!m_vAnimations.empty())
					{
						Animation *pAnimation = m_vAnimations[m_vAnimations.size() - 1];
						lastTime = pAnimation->getTimeEnd() + 1;
					}
					pAnimation = new Animation(lastTime + startTime,lastTime + endTime,true,animationName.c_str());
					m_vAnimations.push_back(pAnimation);
					if (pAnimation)
						m_AnimationMap[animationName] = pAnimation;
				}
				break;
			case 'MBON':
				{
					if (!pAnimation)
						return;
					uint numBones;
					stream.read(&numBones,sizeof(numBones));
					for (uint i = 0;i < numBones;i++)
					{
						int id;
						stream.read(&id,sizeof(id));
						int parentId;
						stream.read(&parentId,sizeof(parentId));
						KeyFrames<Vector3> keyFramesTranslation;
						readKeyFrames<Vector3,Vector3>(&stream,&keyFramesTranslation,lastTime);
						KeyFrames<Quaternion> keyFramesRotation;
						readKeyFrames<Quaternion,Quaternion>(&stream,&keyFramesRotation,lastTime);
						KeyFrames<Vector3> keyFramesScale;
						readKeyFrames<Vector3,Vector3>(&stream,&keyFramesScale,lastTime);

						BoneData *pBone = m_vBones[i];
						if(!pBone)return;

						size_t size = keyFramesTranslation.numKeyFrames();
						for(size_t i = 0;i < size;i++)
						{
							KeyFrame<Vector3>& kf = *keyFramesTranslation.getKeyFrame(i);
							pBone->translation.addKeyFrame(kf);
						}
						size = keyFramesRotation.numKeyFrames();
						for(size_t i = 0;i < size;i++)
						{
							KeyFrame<Quaternion>& kf = *keyFramesRotation.getKeyFrame(i);
							pBone->rotation.addKeyFrame(kf);
						}
						size = keyFramesScale.numKeyFrames();
						for(size_t i = 0;i < size;i++)
						{
							KeyFrame<Vector3>& kf = *keyFramesScale.getKeyFrame(i);
							pBone->scale.addKeyFrame(kf);
						}
						bool b;
						stream.read(&b,sizeof(b));
						stream.read(&b,sizeof(b));
					}
				}
				break;
			}
			pChunk = chunk.nextChunk(pStream);
		}
	}

	bool ModelCodecMz::decode(rkt::Stream *pStream)
	{

		//现在版本是4
		//版本4支持导出时计算aabb


		uint ver = 0;
		rkt::DataChunk chunk;
		rkt::DataChunk::stChunk *pChunk = chunk.beginChunk(pStream);

		size_t sizeVertexBuffer =0;//记录vertexbuffer大小
		size_t sizeIndexBuffer =0;//记录indexbuffer大小
		size_t sizeTexBuffer = 0;//记录纹理buffer大小
		size_t sizeBlendWeightBuffer =0;//记录indexbuffer大小
		size_t sizeBlendIndicesBuffer = 0;//记录纹理buffer大小

		while(pChunk)
		{
			if(pChunk->m_ui32DataSize == 0)
			{
				pChunk = chunk.nextChunk(pStream);
				continue;
			}
			rkt::MemoryStream stream((uchar*)pChunk->m_pData,pChunk->m_ui32DataSize);
			switch(pChunk->m_ui32Type)
			{
			case 'MVER':
				{
					stream.read(&ver,sizeof(ver));
					m_ver = ver;
					if(ver < 1)
					{
						return false;
					}
				}
				break;
			case 'MVTX':
				{
					m_ui32VerticeNum = stream.getLength() / sizeof(_VertexData);
					m_pVertexData = new _VertexData[m_ui32VerticeNum];
					memcpy(m_pVertexData, pChunk->m_pData, pChunk->m_ui32DataSize);
					calcAABB(m_pVertexData, m_ui32VerticeNum, m_AABB,m_BoundingSphere);

					m_pVBPosNormal = m_pRenderSystem->getBufferManager()->createVertexBuffer(12,2 * m_ui32VerticeNum, BU_STATIC_WRITE_ONLY);
					if (!m_pVBPosNormal)
						return false;

					sizeVertexBuffer = 12 * 2 * m_ui32VerticeNum;

					float* pBuffer = (float*)m_pVBPosNormal->lock(0, 0, BL_NORMAL/*BL_DISCARD*/);
					if(pBuffer)
					{
						for(uint i = 0;i < m_ui32VerticeNum;i++)
						{
							*pBuffer++ = m_pVertexData[i].pos[0];
							*pBuffer++ = m_pVertexData[i].pos[1];
							*pBuffer++ = m_pVertexData[i].pos[2];
						}
						for(uint i = 0;i < m_ui32VerticeNum;i++)
						{
							*pBuffer++ = m_pVertexData[i].normal[0];
							*pBuffer++ = m_pVertexData[i].normal[1];
							*pBuffer++ = m_pVertexData[i].normal[2];
						}
						m_pVBPosNormal->unlock();
					}

					m_pVBTexcoord = m_pRenderSystem->getBufferManager()->createVertexBuffer(8,m_ui32VerticeNum,BU_STATIC_WRITE_ONLY);
					if (!m_pVBTexcoord)
						return false;

					sizeTexBuffer = 8 * m_ui32VerticeNum;

					pBuffer = (float*)m_pVBTexcoord->lock(0,0,BL_NORMAL);
					if(pBuffer)
					{
						for(uint i = 0;i < m_ui32VerticeNum;i++)
						{
							*pBuffer++ = m_pVertexData[i].texcoords[0];
							*pBuffer++ = m_pVertexData[i].texcoords[1];
						}
						m_pVBTexcoord->unlock();
					}

					//如果不支持硬件蒙皮就不用创建
					m_pVBBlendWeight = m_pRenderSystem->getBufferManager()->createVertexBuffer(16, m_ui32VerticeNum, BU_STATIC_WRITE_ONLY);
					if (!m_pVBBlendWeight)
						return false;

					sizeBlendWeightBuffer = 16 * m_ui32VerticeNum;

					pBuffer = (float*)m_pVBBlendWeight->lock(0, 0, BL_NORMAL);
					if (pBuffer)
					{
						for(uint i = 0; i < m_ui32VerticeNum; i++)
						{
							*pBuffer++ = m_pVertexData[i].weights[0];
							*pBuffer++ = m_pVertexData[i].weights[1];
							*pBuffer++ = m_pVertexData[i].weights[2];
							*pBuffer++ = m_pVertexData[i].weights[3];
						}
						m_pVBBlendWeight->unlock();
					}

					m_pVBBlendIndices = m_pRenderSystem->getBufferManager()->createVertexBuffer(16, m_ui32VerticeNum, BU_STATIC_WRITE_ONLY);
					if (!m_pVBBlendIndices)
						return false;

					sizeBlendIndicesBuffer = 16 * m_ui32VerticeNum;

					pBuffer = (float*)m_pVBBlendIndices->lock(0, 0, BL_NORMAL);
					if (pBuffer)
					{
						for(uint i = 0; i < m_ui32VerticeNum; i++)
						{
							//*pBuffer++ = BoneIdToFloat(m_pVertexData[i].bones[0]);
							//*pBuffer++ = BoneIdToFloat(m_pVertexData[i].bones[1]);
							//*pBuffer++ = BoneIdToFloat(m_pVertexData[i].bones[2]);
							//*pBuffer++ = BoneIdToFloat(m_pVertexData[i].bones[3]);
							*pBuffer++ = m_pVertexData[i].bones[0];
							*pBuffer++ = m_pVertexData[i].bones[1];
							*pBuffer++ = m_pVertexData[i].bones[2];
							*pBuffer++ = m_pVertexData[i].bones[3];
						}
						m_pVBBlendIndices->unlock();
					}
				}
				break;
			case 'MFAC':
				{
					uint nFace = stream.getLength() / sizeof(MODELFACE);
					MODELFACE* pFace = (MODELFACE *)stream.getBuffer();

					m_pIB = m_pRenderSystem->getBufferManager()->createIndexBuffer(IT_16BIT,nFace * 3,BU_STATIC_WRITE_ONLY);
					if (!m_pIB)
						return false;

					sizeIndexBuffer = 16 * nFace * 3;

					ushort *pBuffer = (ushort *)m_pIB->lock(0,0,BL_NORMAL);
					if(pBuffer)
					{
						for(uint i = 0;i < nFace;i++)
						{
							pBuffer[i * 3 + 0] = pFace[i].index[0];
							pBuffer[i * 3 + 1] = pFace[i].index[1];
							pBuffer[i * 3 + 2] = pFace[i].index[2];
						}
						m_pIB->unlock();
					}
				}
				break;
			case 'MSUB':
				{
					uint nSubMesh = stream.getLength() / sizeof(SUBMESH_V1_3);
					if(nSubMesh)
					{
						SUBMESH_V1_3 *pSubMesh = (SUBMESH_V1_3 *)stream.getBuffer();
						for(uint i = 0;i < nSubMesh;i++)
						{
							SubModel pass;
							pass.indexStart = pSubMesh[i].istart;
							pass.indexCount = pSubMesh[i].icount;
							pass.vertexStart = pSubMesh[i].vstart;
							pass.vertexEnd = pSubMesh[i].vstart + pSubMesh[i].vcount - 1;
							pass.m_matIndex = pSubMesh[i].matId;
							pass.m_bAnimated = pSubMesh[i].animated;
							pass.m_name = pSubMesh[i].name;

							if(pass.m_bAnimated)
								m_bAnimatedGeometry = true; 

							m_vRenderPasses.push_back(pass);
						}
					}
				}
				break;
			case 'MMTX':
				{
					uint numMaterials;
					stream.read(&numMaterials, sizeof(numMaterials));
					for (uint i = 0;i < numMaterials;i++)
					{
						Material mtl;
						Material *pMaterial = &mtl;
						Assert(pMaterial != 0);

						uchar matNameLen;
						stream.read(&matNameLen, sizeof(matNameLen));
						char matName[128];
						stream.read(matName, matNameLen);
						matName[matNameLen] = 0;
						pMaterial->setName(matName);

						bool lighting;
						stream.read(&lighting, sizeof(lighting));

						color ambient;
						stream.read(&ambient, sizeof(ambient));

						color diffuse;
						stream.read(&diffuse, sizeof(diffuse));

						color specular;
						stream.read(&specular, sizeof(specular));

						color emissive;
						stream.read(&emissive, sizeof(emissive));

						bool iTrans;
						stream.read(&iTrans, sizeof(iTrans));

						bool twoSide;
						stream.read(&twoSide, sizeof(twoSide));

						uint nTexture;
						stream.read(&nTexture, sizeof(nTexture));
						for(uint j = 0;j < nTexture;j++)
						{
							MaterialLayer layer;

							layer.m_bTwoSide = twoSide;

							uchar filenameLen;
							stream.read(&filenameLen,sizeof(filenameLen));
							char str[128];
							stream.read(str,filenameLen);
							str[filenameLen] = 0;

							CPath path = m_strName;
							CPath pathP = path.getParentDir();
							if(!pathP.empty())
							{
								pathP.addTailSlash();
								pathP += str;
							}
							else
							{
								pathP = str;
							}
							layer.m_szTexture = pathP.c_str();

							uchar opType;
							stream.read(&opType,sizeof(opType));
							int bm = opType;

							layer.m_blendMode = (BlendMode)bm;
							pMaterial->addLayer(layer);
						}
						m_vMaterials.push_back(mtl);
					}

					size_t size = m_vRenderPasses.size();
					for(size_t j = 0;j < size;j++)
					{
						SubModel& subModel = m_vRenderPasses[j];
						size_t mtlSize = m_vMaterials.size();
						for(size_t k = 0;k < size;k++)
						{
							if(subModel.m_matIndex == k)
							{
								subModel.setMaterial(m_vMaterials[k]);
								break;
							}
						}
					}
				}
				break;
			case 'MANM':
				{
					uint32 nAnims; 
					stream.read(&nAnims,sizeof(nAnims));
					MODELANIMATION *pAnims = (MODELANIMATION *)(stream.getBuffer() + stream.getPosition());

					for(uint i = 0;i < nAnims;i++)
					{
						uchar AnimnameLen; 
						stream.read(&AnimnameLen,sizeof(AnimnameLen));
						char str[64];
						stream.read(str,AnimnameLen);
						str[AnimnameLen] = 0;

						uint startTime,endTime;
						stream.read(&startTime,sizeof(startTime));
						stream.read(&endTime,sizeof(endTime));

						Animation *pAnimation = new Animation(startTime,endTime,true,str);
						//modified by xxh 20091011， 下面那样写不好，会使得m_vAnimations和m_AnimationMap不一致，改为这样。
						if( pAnimation)
						{
							m_vAnimations.push_back(pAnimation);
							if( pAnimation && m_AnimationMap.find(pAnimation->getName() ) == m_AnimationMap.end() )
								m_AnimationMap[pAnimation->getName() ] = pAnimation;
						}
						//m_vAnimations.push_back(pAnimation);
						//if(pAnimation)m_AnimationMap[str] = pAnimation;
					}
				}
				break;
			case 'MBOX': //只有版本4以及4以上才有
				{
					Vector3 vMin,vMax;
					stream.read(vMin.val, sizeof(vMin.val));
					stream.read(vMax.val, sizeof(vMax.val));
					m_AABB.setMinimum(vMin);
					m_AABB.setMaximum(vMax);
					m_BoundingSphere.setCenter( vMin.midPoint(vMax) );
					m_BoundingSphere.setRadius( (vMax-vMin).length() * 0.5);
					m_bCalculatedAABB = true;
				}
				break;
			case 'MBON':
				{
					uint nBones;
					stream.read(&nBones,sizeof(nBones));

					for(uint i = 0;i < nBones;i++)
					{
						int id;
						stream.read(&id,sizeof(id));

						uchar JointnameLen; 
						stream.read(&JointnameLen,sizeof(JointnameLen));
						char str[81];
						stream.read(str,JointnameLen);
						str[JointnameLen] = 0;

						int parent;
						stream.read(&parent,sizeof(parent));
						Vector3 pivot;
						stream.read(&pivot,sizeof(pivot));

						BoneData *pData = new BoneData;

						//读取模型的初始变换
						float _t[3],_r[4],_s[3];
						stream.read(_t, sizeof(float)*3);
						stream.read(_r, sizeof(float)*4);
						stream.read(_s, sizeof(float)*3);
						pData->initTrans = Vector3(_t[0],_t[1],_t[2]);
						pData->initQuat = Quaternion(_r[0],_r[1],_r[2],_r[3]);
						//pData->initQuat = Quaternion(_r[0],-_r[2],_r[1],_r[3]);//按3dmax9的方式，这种方式不适合本引擎
						pData->initScale = Vector3(_s[0],_s[1],_s[2]);


						pData->billboarded = false;
						pData->billboardedLockX = false;
						pData->billboardedLockY =false;
						pData->billboardedLockZ = false;
						pData->cameraAnchored = false;
						pData->dontInheritRotation = false;
						pData->dontInheritScaling = false;
						pData->dontInheritTranslation = false;
						lstrcpyn(pData->name,str,sizeof(pData->name));
						pData->objectId = id;
						pData->parentId = parent;
						pData->pivotPoint = pivot;
						pData->pivotRotation = false;

						//开启插值运算 mod by gxj 2013-8-14
						//pData->translation.setInterpolationType(INTERPOLATION_NONE);
						//pData->rotation.setInterpolationType(INTERPOLATION_NONE);
						//pData->scale.setInterpolationType(INTERPOLATION_NONE);
						pData->translation.setInterpolationType(INTERPOLATION_LINEAR);
						pData->rotation.setInterpolationType(INTERPOLATION_LINEAR);
						pData->scale.setInterpolationType(INTERPOLATION_LINEAR);

						uint nKeyframes;
						stream.read(&nKeyframes,sizeof(nKeyframes));
						for(uint j = 0;j < nKeyframes;j++)
						{
							ModelKeyframeTranslation kf;
							stream.read(&kf,sizeof(kf));

							KeyFrame<Vector3> kfTranslation(kf.time,Vector3(kf.v[0],kf.v[1],kf.v[2]));

							pData->translation.addKeyFrame(kfTranslation);
						}
						stream.read(&nKeyframes,sizeof(nKeyframes));
						for(uint j = 0;j < nKeyframes;j++)
						{
							ModelKeyframeRotation kf;
							stream.read(&kf,sizeof(kf));

							KeyFrame<Quaternion> kfRotation(kf.time,Quaternion(kf.q[0],kf.q[1],kf.q[2],kf.q[3]));

							pData->rotation.addKeyFrame(kfRotation);
						}
						stream.read(&nKeyframes,sizeof(nKeyframes));
						for(uint j = 0;j < nKeyframes;j++)
						{
							ModelKeyframeScale kf;
							stream.read(&kf,sizeof(kf));

							KeyFrame<Vector3> kfScale(kf.time,Vector3(kf.v[0],kf.v[1],kf.v[2]));

							pData->scale.addKeyFrame(kfScale);
						}
						m_vBones.push_back(pData);

						//add by yhc
						//直接读取矩阵
						//stream.read(&nKeyframes,sizeof(nKeyframes));
						//for(uint j = 0;j < nKeyframes;j++)
						//{
						//	ModelKeyframeTM mktm;
						//	stream.read(&mktm,sizeof(mktm));
						//	pData->m_TMVector.push_back(mktm);
						//	
						//	Matrix4 mtest(( mktm.tm )[0][0],( mktm.tm )[0][1],( mktm.tm )[0][2],( mktm.tm )[0][3],( mktm.tm )[1][0],( mktm.tm )[1][1],( mktm.tm )[1][2],( mktm.tm )[1][3],
						//				( mktm.tm )[2][0],( mktm.tm )[2][1],( mktm.tm )[2][2],( mktm.tm )[2][3],( mktm.tm )[3][0],( mktm.tm )[3][1],( mktm.tm )[3][2],( mktm.tm )[3][3]);


						//	//缩放
						//	Vector3 s;
						//	s.x = Vector3(( mktm.tm )[0][0],( mktm.tm )[1][0],( mktm.tm )[2][0]).length();
						//	s.y = Vector3(( mktm.tm )[0][1],( mktm.tm )[1][1],( mktm.tm )[2][1]).length();
						//	s.z = Vector3(( mktm.tm )[0][2],( mktm.tm )[1][2],( mktm.tm )[2][2]).length();
						//	mtest.getScale(s);
						//	KeyFrame<Vector3> kfScale(mktm.time,s);
						//	pData->scale.addKeyFrame(kfScale);

						//	//平移
						//	Vector3 tran = mtest.getTrans();
						//	KeyFrame<Vector3> kfTranslation(mktm.time,tran);
						//	pData->translation.addKeyFrame(kfTranslation);
						//	
						//	//旋转
						//	Quaternion q = mtest.extractQuaternion();
						//	KeyFrame<Quaternion> kfRotation(mktm.time,q);
						//	pData->rotation.addKeyFrame(kfRotation);
						//}

						Stream *pStream = &stream;
						bool hasRibbonSystem;
						bool hasParticleSystem;
						stream.read(&hasRibbonSystem,sizeof(hasRibbonSystem));
						stream.read(&hasParticleSystem,sizeof(hasParticleSystem));
						if(hasRibbonSystem)
						{
							RibbonEmitterData *pData = new RibbonEmitterData;
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

							pData->visibility.setStaticData(visible);
							pData->heightAbove.setStaticData(above);
							pData->heightBelow.setStaticData(below);
							pData->emissionRate = edgePerSecond;
							pData->lifeSpan = edgeLife;
							pData->gravity = gravity;
							pData->rows = rows;
							pData->columns = cols;
							pData->textureSlot.setStaticData(slot);
							pData->color.setStaticData(color);
							pData->alpha.setStaticData(alpha);
							pData->filterMode = (BlendMode)blendMode;

							CPath path = m_strName;
							CPath pathP = path.getParentDir();
							if(!pathP.empty())
							{
								pathP.addTailSlash();
								pathP += filename;
							}
							pData->textureFilename = pathP.c_str();

							readKeyFrames<float,bool>(pStream,&pData->visibility);
							readKeyFrames<float,float>(pStream,&pData->heightAbove);
							readKeyFrames<float,float>(pStream,&pData->heightBelow);
							readKeyFrames<int,short>(pStream,&pData->textureSlot);
							readKeyFrames<Color3,Color3>(pStream,&pData->color);
							readKeyFrames<float,float>(pStream,&pData->alpha);

							pData->visibility.setInterpolationType(INTERPOLATION_NONE);
							pData->heightAbove.setInterpolationType(INTERPOLATION_LINEAR);
							pData->heightBelow.setInterpolationType(INTERPOLATION_LINEAR);
							pData->textureSlot.setInterpolationType(INTERPOLATION_NONE);
							pData->color.setInterpolationType(INTERPOLATION_LINEAR);
							pData->alpha.setInterpolationType(INTERPOLATION_LINEAR);

							pData->boneObjectId = id;
							m_vRibbonEmitters.push_back(pData);
						}
						if(hasParticleSystem)
						{
							bool visible;
							pStream->read(&visible,sizeof(visible));
							float speed;
							pStream->read(&speed,sizeof(speed));
							float variation;
							pStream->read(&variation,sizeof(variation));
							float coneAngle;
							pStream->read(&coneAngle,sizeof(coneAngle));
							float gravity;
							pStream->read(&gravity,sizeof(gravity));
							float life;
							pStream->read(&life,sizeof(life));
							float emissionRate;
							pStream->read(&emissionRate,sizeof(emissionRate));
							float width;
							pStream->read(&width,sizeof(width));
							float length;
							pStream->read(&length,sizeof(length));
							short blendMode;
							pStream->read(&blendMode,sizeof(blendMode));
							short rows;
							pStream->read(&rows,sizeof(rows));
							short cols;
							pStream->read(&cols,sizeof(cols));
							float tailLength;
							pStream->read(&tailLength,sizeof(tailLength));
							float timeMiddle;
							pStream->read(&timeMiddle,sizeof(timeMiddle));
							Color3 colorStart;
							pStream->read(&colorStart,sizeof(colorStart));
							Color3 colorMiddle;
							pStream->read(&colorMiddle,sizeof(colorMiddle));
							Color3 colorEnd;
							pStream->read(&colorEnd,sizeof(colorEnd));
							Vector3 alpha;
							pStream->read(&alpha,sizeof(alpha));
							Vector3 scale;
							pStream->read(&scale,sizeof(scale));
							typedef short short3[3];
							short3 headLifeSpan;
							pStream->read(&headLifeSpan,sizeof(headLifeSpan));
							short3 headDecay;
							pStream->read(&headDecay,sizeof(headDecay));
							short3 tailLifeSpan;
							pStream->read(&tailLifeSpan,sizeof(tailLifeSpan));
							short3 tailDecay;
							pStream->read(&tailDecay,sizeof(tailDecay));
							bool head,tail,unshaded,unfogged;
							pStream->read(&head,sizeof(head));
							pStream->read(&tail,sizeof(tail));
							pStream->read(&unshaded,sizeof(unshaded));
							pStream->read(&unfogged,sizeof(unfogged));
							uchar textureFilenameLen;
							stream.read(&textureFilenameLen,sizeof(textureFilenameLen));
							char filename[256];
							stream.read(filename,textureFilenameLen);
							filename[textureFilenameLen] = 0;

							ParticleEmitter2Data *pData = new ParticleEmitter2Data;
							pData->filterMode = (BlendMode)blendMode;
							pData->speed.setStaticData(speed);
							pData->variation.setStaticData(variation);
							pData->latitudeMin.setStaticData(coneAngle);
							pData->gravity.setStaticData(gravity);
							pData->visibility.setStaticData(visible);
							pData->emissionRate.setStaticData(emissionRate);
							pData->width.setStaticData(width);
							pData->length.setStaticData(length);

							CPath path = m_strName;
							CPath pathP = path.getParentDir();
							if(!pathP.empty())
							{
								pathP.addTailSlash();
								pathP += filename;
							}
							pData->textureFilename = pathP.c_str();

							pData->segmentColor1 = colorStart;
							pData->segmentColor2 = colorMiddle;
							pData->segmentColor3 = colorEnd;

							pData->alpha = alpha;
							pData->particleScaling = scale;
							pData->headLifeSpan = Vector3(headLifeSpan[0],headLifeSpan[1],headLifeSpan[2]);
							pData->headDecay = Vector3(headDecay[0],headDecay[1],headDecay[2]);
							pData->tailLifeSpan = Vector3(tailLifeSpan[0],tailLifeSpan[1],tailLifeSpan[2]);
							pData->tailDecay = Vector3(tailDecay[0],tailDecay[1],tailDecay[2]);

							pData->rows = rows;
							pData->columns = cols;

							pData->time = timeMiddle;
							pData->lifeSpan = life;
							pData->tailLength = tailLength;

							pData->head = head;
							pData->tail = tail;
							pData->unshaded = unshaded;
							pData->unfogged = unfogged;

							readKeyFrames<float,bool>(pStream,&pData->visibility);
							readKeyFrames<float,float>(pStream,&pData->speed);
							readKeyFrames<float,float>(pStream,&pData->variation);
							readKeyFrames<float,float>(pStream,&pData->latitudeMin);
							readKeyFrames<float,float>(pStream,&pData->gravity);
							readKeyFrames<float,float>(pStream,&pData->emissionRate);
							readKeyFrames<float,float>(pStream,&pData->width);
							readKeyFrames<float,float>(pStream,&pData->length);

							pData->visibility.setInterpolationType(INTERPOLATION_NONE);
							pData->speed.setInterpolationType(INTERPOLATION_LINEAR);
							pData->variation.setInterpolationType(INTERPOLATION_LINEAR);
							pData->latitudeMin.setInterpolationType(INTERPOLATION_LINEAR);
							pData->gravity.setInterpolationType(INTERPOLATION_LINEAR);
							pData->emissionRate.setInterpolationType(INTERPOLATION_LINEAR);
							pData->width.setInterpolationType(INTERPOLATION_LINEAR);
							pData->length.setInterpolationType(INTERPOLATION_LINEAR);

							pData->boneObjectId = id;

							m_vParticleEmitters.push_back(pData);
						}
					}
				}
				break;
			}
			pChunk = chunk.nextChunk(pStream);
		}

		//读取非编

		m_memorySize += sizeof(*this);
		m_memorySize += m_vRenderPasses.size() * sizeof(SubModel);
		m_memorySize += m_vAnimations.size() * sizeof(Animation);
		m_memorySize += m_vParticleEmitters.size() * sizeof(ParticleEmitter2Data);
		m_memorySize += m_vRibbonEmitters.size() * sizeof(RibbonEmitterData);
		size_t size = m_vBones.size();
		for(size_t i = 0;i < size;i++)
		{
			m_memorySize += m_vBones[i]->getMemorySize();
		}
		size = m_vMaterials.size();
		for(size_t i = 0;i < size;i++)
		{
			m_memorySize += m_vMaterials[i].getMemorySize();
		}
		m_memorySize += m_ui32VerticeNum * sizeof(_VertexData);

		//将模型占有的agp，显卡内存也计算在内
		m_memorySize += sizeVertexBuffer;//计算vertexbuffer大小
		m_memorySize += sizeIndexBuffer;//计算indexbuffer大小
		m_memorySize += sizeTexBuffer;//计算纹理buffer大小
		m_memorySize += sizeBlendWeightBuffer;//计算indexbuffer大小
		m_memorySize += sizeBlendIndicesBuffer;//计算纹理buffer大小
		if( m_vAnimations.size() > 0 )//计算动画vertexbuffer的大小，没有动画texbuffer
		{
			//人物的动作不好估计，如果说所有的动作都会产生，则需占用大量内存，而实际情况不是这样的
			//因此做折中设定，一个一般动作，一个攻击动作。
			//int _frames = m_vAnimations[m_vAnimations.size() -1]->getTimeEnd() - m_vAnimations[0]->getTimeStart();
			int _frames = (m_vAnimations[m_vAnimations.size() -1]->getTimeEnd() - m_vAnimations[0]->getTimeStart() ) *
				static_cast<float>( (m_vAnimations.size()>=2 ?2:1) ) /static_cast<float>( m_vAnimations.size() );
			_frames = _frames *30 /1000;
			m_memorySize += sizeVertexBuffer * _frames;
		}

		return true;
	}
	
	bool ModelCodecMz::write(string filename,float fCopress)
	{

		Stream *pDataStream;
		DataChunk writeChunk;

		//最新版本是7，把所有旧的版本转换为版本4
		//写 'MVER'
		writeChunk.beginChunk('MVER', &pDataStream);
		uint ver = 4 ;//在导出插件里面计算aabb
		pDataStream->write(&ver,sizeof(ver));
		writeChunk.endChunk();


		//写 'MVTX'
		if( m_pVertexData != 0 )
		{
			writeChunk.beginChunk('MVTX', &pDataStream);
			pDataStream->write(m_pVertexData, m_ui32VerticeNum * sizeof(_VertexData) );
			writeChunk.endChunk();
		}

		if( m_pIB != 0 &&  m_pIB->getNumIndices() > 0)
		{
			uint nFace = m_pIB->getNumIndices() / 3;
			writeChunk.beginChunk('MFAC', &pDataStream);
			MODELFACE * pFace = new MODELFACE[nFace];
			ushort * pBuffer = (ushort*)m_pIB->lock(0,0,BL_NORMAL);
			if( pBuffer != 0)
			{
				for( uint i = 0; i < nFace; i++)
				{
					pFace[i].index[0] = pBuffer[i*3 + 0];
					pFace[i].index[1] = pBuffer[i*3 + 1];
					pFace[i].index[2] = pBuffer[i*3 + 2];
				}
				m_pIB->unlock();
				pDataStream->write(pFace,nFace * sizeof(MODELFACE));
			}
			delete[] pFace;
			pFace = 0;
			writeChunk.endChunk();
		}

		//写'MSUB'
		uint nSubMesh = m_vRenderPasses.size();
		if( nSubMesh > 0)
		{
			writeChunk.beginChunk('MSUB', &pDataStream);
			SUBMESH_V1_3 * pSubMesh = new SUBMESH_V1_3[nSubMesh];
			for( uint i=0; i < nSubMesh ; i++)
			{
				pSubMesh[i].istart = m_vRenderPasses[i].indexStart;
				pSubMesh[i].icount = m_vRenderPasses[i].indexCount;
				pSubMesh[i].vstart = m_vRenderPasses[i].vertexStart;
				pSubMesh[i].vcount = m_vRenderPasses[i].vertexEnd + 1 - pSubMesh[i].vstart;
				pSubMesh[i].matId = m_vRenderPasses[i].m_matIndex;
				pSubMesh[i].animated = m_vRenderPasses[i].m_bAnimated;
				lstrcpyn(pSubMesh[i].name, m_vRenderPasses[i].m_name.c_str(), sizeof(pSubMesh[i].name));
			}
			pDataStream->write(pSubMesh, nSubMesh*(sizeof(SUBMESH_V1_3)) );
			delete [] pSubMesh;
			pSubMesh = 0;
			writeChunk.endChunk();
		}
		
		//包围盒
		writeChunk.beginChunk('MBOX',&pDataStream);
		Vector3 _vMin,_vMax;
		_vMin = m_AABB.getMinimum();
		_vMax = m_AABB.getMaximum();
		pDataStream->write(_vMin.val,sizeof(float)*3);
		pDataStream->write(_vMax.val,sizeof(float)*3);
		writeChunk.endChunk();

		//写'MMTX'
		uint numMaterials = m_vMaterials.size();
		if( numMaterials > 0)
		{
			writeChunk.beginChunk('MMTX', &pDataStream);
			pDataStream->write( &numMaterials, sizeof(numMaterials));
			for( uint i =0 ; i< numMaterials; i++)
			{
				uchar matNameLen = m_vMaterials[i].getName().size();
				pDataStream->write(&matNameLen, sizeof(matNameLen));
				pDataStream->write( m_vMaterials[i].getName().c_str(), matNameLen);
				bool lighting = m_vMaterials[i].getLayer(0)->m_bUnshaded ? true:false;
				pDataStream->write( &lighting, sizeof(lighting) );
				color ambient(0,0,0,0);
				pDataStream->write( &ambient, sizeof(ambient) );
				color diffuse(0,0,0,0);
				pDataStream->write( &diffuse, sizeof(diffuse) );
				color specular(0,0,0,0);
				pDataStream->write( &specular, sizeof(specular) );
				color emissive(0,0,0,0);
				pDataStream->write( &emissive, sizeof(emissive) );
				bool iTrans = false;
				pDataStream->write( &iTrans, sizeof(iTrans) );
				bool twoSide = false;	
				for( int ii = 0; ii < m_vMaterials[i].numLayers(); ii ++)
				{
					twoSide = m_vMaterials[i].getLayer(ii)->m_bTwoSide;
					break;
				}
				pDataStream->write( &twoSide, sizeof(twoSide) );


				uint nTexture = m_vMaterials[i].numLayers();
				pDataStream->write( &nTexture, sizeof(nTexture) );
				for( uint ii = 0; ii < nTexture; ii ++)
				{
					CPath path = m_vMaterials[i].getLayer(ii)->m_szTexture;
					string filename = path.getFileName().c_str();
					uchar filenameLen = filename.size();
					pDataStream->write(&filenameLen,sizeof(filenameLen));
					pDataStream->write(filename.c_str(), filenameLen);
					uchar opType = static_cast<uchar>(static_cast<int>(m_vMaterials[i].getLayer(ii)->m_blendMode ));
					pDataStream->write(&opType, sizeof(opType));
				}
			}
			writeChunk.endChunk();		
		}

		//写 'MANM'
		uint32 nAnims = m_vAnimations.size();

		//add by yhc 2010.5.10
		//修正前一个动作的结束时间和后一个动作的开始时间一样的饿问题
		bool bFixWrongTime = false;

		if( nAnims > 0)
		{
			if( nAnims>1 && m_vAnimations[0]->getTimeEnd()==m_vAnimations[1]->getTimeStart() )
				bFixWrongTime = true;

			writeChunk.beginChunk('MANM', &pDataStream);
			pDataStream->write( &nAnims, sizeof(nAnims) );
			for( uint i =0; i < nAnims; i++)
			{
				string animname = m_vAnimations[i]->getName();
				uchar animnameLen = animname.size();
				pDataStream->write(&animnameLen, sizeof( animnameLen) );
				pDataStream->write(animname.c_str(), animnameLen);
				uint startTime,endTime;
				startTime = m_vAnimations[i]->getTimeStart();
				if(bFixWrongTime)
					startTime += i*33;

				endTime = m_vAnimations[i]->getTimeEnd();
				if(bFixWrongTime)
					endTime += i*33;

				pDataStream->write(&startTime, sizeof(startTime));
				pDataStream->write(&endTime, sizeof(endTime));
			}
			writeChunk.endChunk();
		}

		//写 'MBON'
		uint nBones = m_vBones.size();
		if( nBones > 0)
		{
			writeChunk.beginChunk('MBON', &pDataStream);
			pDataStream->write( &nBones, sizeof(nBones) );
			for( uint i = 0; i < nBones; i++)
			{
				int id  = m_vBones[i]->objectId;
				pDataStream->write( &id, sizeof(id));
				string Jointname = m_vBones[i]->name;
				uchar JointnameLen = Jointname.size();
				pDataStream->write( &JointnameLen, sizeof(JointnameLen) );
				pDataStream->write( Jointname.c_str(), JointnameLen);
				int parent = m_vBones[i]->parentId;
				pDataStream->write( &parent, sizeof(parent) );
				Vector3 pivot = m_vBones[i]->pivotPoint;
				pDataStream->write( &pivot, sizeof(pivot) );

				//写骨骼的初始变换矩阵
				pDataStream->write(m_vBones[i]->initTrans.val,sizeof(float)*3);
				float _r[4];
				_r[0] = m_vBones[i]->initQuat.x;
				_r[1] = m_vBones[i]->initQuat.y;
				_r[2] = m_vBones[i]->initQuat.z;
				_r[3] = m_vBones[i]->initQuat.w;
				pDataStream->write(_r, sizeof(float)*4);
				pDataStream->write(m_vBones[i]->initScale.val, sizeof(float)*3);

				KeyFrames<Vector3> translation;
				KeyFrames<Quaternion> rotation;
				KeyFrames<Vector3> scale;
				
				//去掉重复和多余的keyframe 
				CompressPos(m_vBones[i]->translation,translation,fCopress);
				
				uint nKeyFrames = translation.numKeyFrames();
				//修正时间
				if(bFixWrongTime)
				{
					int nCurAniIndex=0;
					for( uint ii = 0; ii < nKeyFrames; ii++)
					{
						KeyFrame<Vector3> * kf =  translation.getKeyFrame(ii);
						if( kf->time>m_vAnimations[nCurAniIndex]->getTimeEnd() )
							nCurAniIndex++;
						kf->time += nCurAniIndex*33;
					}

				}

				pDataStream->write( &nKeyFrames, sizeof(nKeyFrames) );
				for( uint ii = 0; ii < nKeyFrames; ii++)
				{
					ModelKeyframeTranslation mkft;
					mkft.time = translation.getKeyFrame(ii)->time;
					mkft.v[0] = translation.getKeyFrame(ii)->v[0];
					mkft.v[1] = translation.getKeyFrame(ii)->v[1];
					mkft.v[2] = translation.getKeyFrame(ii)->v[2];
					pDataStream->write( &mkft, sizeof(mkft) );
				} 
				
				CompressQua(m_vBones[i]->rotation,rotation,fCopress);
				nKeyFrames = rotation.numKeyFrames();

				//修正时间
				if(bFixWrongTime)
				{
					int nCurAniIndex=0;
					for( uint ii = 0; ii < nKeyFrames; ii++)
					{
						KeyFrame<Quaternion> * kf =  rotation.getKeyFrame(ii);
						if( kf->time>m_vAnimations[nCurAniIndex]->getTimeEnd() )
							nCurAniIndex++;
						kf->time += nCurAniIndex*33;
					}
				}

				pDataStream->write( &nKeyFrames, sizeof(nKeyFrames) );
				for( uint ii = 0; ii < nKeyFrames; ii++)
				{
					ModelKeyframeRotation mkfr;
					mkfr.time = rotation.getKeyFrame(ii)->time;
					mkfr.q[0] = rotation.getKeyFrame(ii)->v.x;
					mkfr.q[1] = rotation.getKeyFrame(ii)->v.y;
					mkfr.q[2] = rotation.getKeyFrame(ii)->v.z;
					mkfr.q[3] = rotation.getKeyFrame(ii)->v.w;
					pDataStream->write( &mkfr, sizeof(mkfr) );
				} 
				
				CompressPos(m_vBones[i]->scale,scale,fCopress);
				nKeyFrames = scale.numKeyFrames();
				//修正时间
				if(bFixWrongTime)
				{
					int nCurAniIndex=0;
					for( uint ii = 0; ii < nKeyFrames; ii++)
					{
						KeyFrame<Vector3> * kf =  scale.getKeyFrame(ii);
						if( kf->time>m_vAnimations[nCurAniIndex]->getTimeEnd() )
							nCurAniIndex++;
						kf->time += nCurAniIndex*33;
					}

				}
				pDataStream->write( &nKeyFrames, sizeof( nKeyFrames) );
				for( uint ii = 0; ii < nKeyFrames; ii ++)
				{
					ModelKeyframeScale mkfs;
					mkfs.time = scale.getKeyFrame(ii)->time;
					mkfs.v[0] = scale.getKeyFrame(ii)->v[0];
					mkfs.v[1] = scale.getKeyFrame(ii)->v[1];
					mkfs.v[2] = scale.getKeyFrame(ii)->v[2];
					pDataStream->write( &mkfs, sizeof(mkfs) );
				}
				//有没有粒子和带子
				bool	hasParticleSystem = false;
				bool	hasRibbonSystem = false;
				pDataStream->write(&hasRibbonSystem,sizeof(hasParticleSystem));
				pDataStream->write(&hasParticleSystem,sizeof(hasParticleSystem));
			}
			writeChunk.endChunk();		
		}

		writeChunk.save(filename.c_str());

		return true;
	}

	ModelCodecMz::~ModelCodecMz()
	{
		safeRelease(m_pVBs);
		m_vMaterials.clear();
		STLDeleteSequence(m_vParticleEmitters);
		STLDeleteSequence(m_vRibbonEmitters);
		STLDeleteSequence(m_vBones);
		STLDeleteSequence(m_vAnimations);
		safeRelease(m_pVBTexcoord);
		safeRelease(m_pVBPosNormal);
		safeRelease(m_pVBBlendWeight);
		safeRelease(m_pVBBlendIndices);
		safeRelease(m_pIB);
		safeDeleteArray(m_pVertexData);
	}

	ModelCodecMz::ModelCodecMz(const std::string& name,IRenderSystem* pRenderSystem)
		: ManagedItem(name.c_str()),
		m_pRenderSystem(pRenderSystem),
		m_pVertexData(0),
		m_pVBTexcoord(0),
		m_pIB(0),
		m_ui32VerticeNum(0),
		m_bAnimatedGeometry(false),
		m_memorySize(0),
		m_pVBPosNormal(0),
		m_pVBBlendWeight(0),
		m_pVBBlendIndices(0),
		m_bCalculatedAABB(false),
		m_pVBs(0)
	{
	}

	void  ModelCodecMz::calcAABB(_VertexData *pVtx,uint num,AABB& aabb,Sphere& sphere)
	{
		if (!num)
			return;

		Vector3 vMin = Vector3(pVtx[0].pos[0],pVtx[0].pos[1],pVtx[0].pos[2]);
		Vector3 vMax = vMin;

		for(uint i = 1;i < num;i++)
		{
			Vector3 v = Vector3(pVtx[i].pos[0],pVtx[i].pos[1],pVtx[i].pos[2]);
			Vector3	*p = &v;

			if(p->x > vMax.x)vMax.x = p->x;
			if(p->y > vMax.y)vMax.y = p->y;
			if(p->z > vMax.z)vMax.z = p->z;
			if(p->x < vMin.x)vMin.x = p->x;
			if(p->y < vMin.y)vMin.y = p->y;
			if(p->z < vMin.z)vMin.z = p->z;
		}
		aabb.setExtents(vMin,vMax);
		sphere.setCenter(vMin.midPoint(vMax));
		sphere.setRadius((vMax - vMin).length() * 0.5f);
	}

	size_t ModelCodecMz::getMomorySize()
	{
		return m_memorySize;
	}

	void ModelCodecMz::release()
	{
		delete this;
	}
}