
	/*! \addtogroup RenderEngine
	*  ��Ⱦ����
	*  @{
	*/
#ifndef __MzHeader_H__
#define __MzHeader_H__

struct Vector2
{
	float d_x, d_y;
	Vector2(void) { d_x = d_y = 0.f; }
	Vector2(float x, float y) : d_x(x), d_y(y) {}
};

struct Vector3
{
	float d_x, d_y, d_z;
	Vector3(void) { d_x = d_y = d_z = 0.f; }
	Vector3(float x, float y, float z) : d_x(x), d_y(y), d_z(z) {}
};

struct Quaternion
{
	float d_x, d_y, d_z, d_w;
	Quaternion(void) { d_x = d_y = d_z = d_w = 0.f; }
	Quaternion(float x, float y, float z, float w) : d_x(x), d_y(y), d_z(z), d_w(w) {}
};

struct Matrix4
{
    /// The matrix entries, indexed by [row][col].
    union {
        float m[4][4];
        float _m[16];
    };
};

struct Color3
{
	union {
		struct {
			float r,g,b;
		};
		float val[3];
	};
};

typedef struct tranTag
{
	float m_mat[4][3];
} tranMatrix;

typedef struct _modelversion 
{
	uint32 nVersion;
} MODELVERSION;

typedef struct _submesh_v1_3
{
	uint16 matId;			//����id
	uint16 vstart;			// first vertex
	uint16 vcount;			// num vertices
	uint16 istart;			// first index
	uint16 icount;			// num indices
	bool		animated;
	char name[256];			// ��ģ������
} SUBMESH_V1_3;

/// Vertex
struct _VertexData
{
	float pos[3];
	float normal[3];
	float color[4];
	float weights[4];
	uint8 bones[4];
	float texcoords[2];
};

typedef struct _modelface
{
	int index[3];
} MODELFACE;

typedef struct _modelanimation
{
	std::string name;	// ��������
	uint32 startTime;
	uint32 endTime;
	uint32 loopType;
} MODELANIMATION;

struct ModelKeyframeTranslation
{
	int time;
	float v[3];
};

struct ModelKeyframeRotation
{
	int time;
	float q[4];
};

struct ModelKeyframeScale
{
	int time;
	float v[3];
};

typedef struct _modelbone
{
	int id;	
	int parent;		// ���ڵ��id
	std::string name;
	tranMatrix tran;

	std::vector<ModelKeyframeTranslation> keyframesTranslation;
	std::vector<ModelKeyframeRotation> keyframesRotation;
	std::vector<ModelKeyframeScale> keyframesScale;
} MODELBONE;


struct color
{	
	color(){}
	color(float ca,float cr,float cg,float cb){ a = ca; r = cr; g = cg; b = cb;}
	color &operator =(const color &c) {  r=c.r; g=c.g; b=c.b; a=c.a; return *this; 	}
	float a,r,g,b;
};

typedef struct _texture
{
	std::string name;
	uchar opType;
}TEXTURE;

typedef struct _material
{
	std::string name;
	bool  lighting;
	color ambient;
	color diffuse;
	color specular;
	color emissive;
	bool  isTransparent;
	bool  twoSide;
	std::vector<TEXTURE> textures;
}MATERIAL;

inline float BoneIdToFloat(uint8 id)
{
	return id * 16.f + ( ( id * 16 ) % 64 ) / 64.f;
}

typedef struct _bonedata
{
	std::string name;
	Vector3 initTrans;
	Quaternion initQuat;
	Vector3 initScale;
	int objectId;
	int parentId;
	
	bool dontInheritTranslation;
	bool dontInheritRotation;
	bool dontInheritScaling;
	bool billboarded;
	bool billboardedLockX;
	bool billboardedLockY;
	bool billboardedLockZ;
	bool cameraAnchored;

	Vector3 pivotPoint;
	bool pivotRotation;

	Matrix4	precomputeMtx;

	std::vector<ModelKeyframeTranslation> keyframesTranslation;
	std::vector<ModelKeyframeRotation> keyframesRotation;
	std::vector<ModelKeyframeScale> keyframesScale;

} BoneData;

#endif
	/** @} */