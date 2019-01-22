#ifndef __ModelCodecMz_H__
#define __ModelCodecMz_H__

#include "RenderEngine/Manager.h"
#include "SubModel.h"
#include "RenderEngine/MzHeader.h"

namespace rkt
{
	class ModelCodecMz :
		public ManagedItem,
		public IModel
	{
	public:
		virtual void release();

		/** 获得文件名
		@return 文件名
		*/
		virtual const std::string& getFileName() ;

		enModelClass GetModelClass() const {return MC_MZ;}

		/** 获得包围盒数据
		@return 模型包围盒
		*/
		virtual const AABB&	getAABB() ;

		/** 获得包围球数据
		@return 模型包围球
		*/
		virtual const Sphere& getBoudingSphere() ;

		/** 获得模型动画数目
		@return 模型动画数
		*/
		virtual uint	getAnimationCount() ;

		/** 获得某个动画
		@param index 下标
		@return 动画指针
		*/
		virtual Animation* getAnimation(uint index);

		/** 是否拥有某个动画
		@param animation 动画名称
		@see enAnimation
		@return 如果存在animation动画则返回此动画数据，否则返回0
		*/
		virtual Animation* hasAnimation(const std::string& animation);

		virtual Animation* randomAnimation();

		virtual uint	numBones();

		virtual BoneData*	getBone(uint index);

		/** 获得子模型的数量
		@return 子模型的数量
		*/
		virtual uint getNumSubModels() ;

		/** 获得子模型指针
		@param name 子模型名称
		@return 子模型指针
		*/
		virtual ISubModel*	getSubModel(const char* name);

		/** 根据下标来获得子模型指针
		@param name 子模型下标
		@return 子模型指针
		*/
		virtual ISubModel*	getSubModel(int index);

		virtual IIndexBuffer*	getIndexBuffer();

		virtual IVertexBuffer*	getTexcoordBuffer();

		virtual IVertexBuffer*	getVBPosNormal() {return m_pVBPosNormal;}

		virtual IVertexBuffer*	getVBBlendWeight() {return m_pVBBlendWeight;}

		virtual IVertexBuffer*	getVBBlendIndices() {return m_pVBBlendIndices;}

		/**获得面数
		*/
		virtual uint getNumFaces();

		/**获得顶点数
		*/
		virtual uint getNumVertices();

		/**获得版本号
		*/
		virtual uint getVer();

		virtual IVertexBuffer* vertexBlend(Skeleton *pSkeleton,const AnimationTime *pTime);

		//根据动画时间来获取定点
		virtual IVertexBuffer* getVertexByAniTime(const AnimationTime *pTime);


		virtual uint numParticleSystem();

		virtual ParticleEmitter2Data*	getParticleSystemData(uint index);

		virtual uint numRibbonSystem();

		virtual RibbonEmitterData*	getRibbonSystemData(uint index);

		virtual uint numMmParticleSystem();
		
		virtual MmParticleSystemData* getMmParticleSystem(uint index);

		/**获得模型占用的内存
		*/
		virtual size_t getMomorySize();
		
		//用于将模型写到文件
		virtual bool write(string filename,float fCopress);
	public:
		virtual ~ModelCodecMz();
		ModelCodecMz(const std::string& name,IRenderSystem* pRenderSystem);

	public:
		const char*	getType() const;
		bool decode(rkt::Stream *pStream);

		void loadFeiBian(rkt::Stream *pStream,const std::string& animationName);

	private:
		IRenderSystem*	m_pRenderSystem;
		std::vector<SubModel> m_vRenderPasses;
		//Textures
		std::vector<Animation*>		m_vAnimations;
		std::map<std::string,Animation*>	m_AnimationMap;
		//VertexBuffer
		IVertexBuffer*			m_pVBTexcoord;
		uint					m_ui32VerticeNum;
		//IndexBuffer
		IIndexBuffer*			m_pIB;
		IVertexBuffer*			m_pVBPosNormal;
		IVertexBuffer*			m_pVBBlendWeight;
		IVertexBuffer*			m_pVBBlendIndices;
		//BoundingVolume
		AABB					m_AABB;
		Sphere					m_BoundingSphere;

		//Skeleton
		std::vector<BoneData*>	m_vBones;

		//Materials
		std::vector<Material>	m_vMaterials;

		//Ribbon
		std::vector<RibbonEmitterData*> m_vRibbonEmitters;

		//Particle
		std::vector<ParticleEmitter2Data*> m_vParticleEmitters;

		uint			m_ver;

		//是否计算了aabb
		bool			m_bCalculatedAABB;

		IVertexBuffer*	m_pVBs;

		//Momory size
		size_t			m_memorySize;

	private:

		_VertexData*	m_pVertexData;

		bool			m_bAnimatedGeometry;

	private:
		void calcAABB(_VertexData *pVtx,uint num,AABB& aabb,Sphere& sphere);
	};
}

#endif