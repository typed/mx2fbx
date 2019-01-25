
	/*! \addtogroup RenderEngine
	*  渲染引擎
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
	inline Vector3 operator - ( const Vector3& rkVector ) const
	{
		Vector3 kDiff;
		kDiff.d_x = d_x - rkVector.d_x;
		kDiff.d_y = d_y - rkVector.d_y;
		kDiff.d_z = d_z - rkVector.d_z;
		return kDiff;
	}
	inline Vector3 operator + ( const Vector3& rkVector ) const
	{
		Vector3 kDiff;
		kDiff.d_x = d_x + rkVector.d_x;
		kDiff.d_y = d_y + rkVector.d_y;
		kDiff.d_z = d_z + rkVector.d_z;
		return kDiff;
	}
};

struct Quaternion
{
	float x, y, z, w;
	Quaternion(void) { x = y = z = w = 0.f; }
	Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
	inline Quaternion operator* (const Quaternion& rkQ) const
	{
		// NOTE:  Multiplication is not generally commutative, so in most
		// cases p*q != q*p.

		return Quaternion
			(
			w * rkQ.x + x * rkQ.w + y * rkQ.z - z * rkQ.y,
			w * rkQ.y + y * rkQ.w + z * rkQ.x - x * rkQ.z,
			w * rkQ.z + z * rkQ.w + x * rkQ.y - y * rkQ.x,
			w * rkQ.w - x * rkQ.x - y * rkQ.y - z * rkQ.z
			);
	}
	inline Quaternion Quaternion::Inverse () const
	{
		float fNorm = w*w+x*x+y*y+z*z;
		if ( fNorm > 0.f )
		{
			float fInvNorm = 1.f / fNorm;
			return Quaternion(-x*fInvNorm,-y*fInvNorm,-z*fInvNorm,w*fInvNorm);
		}
		else
		{
			// return an invalid result to flag the error
			return Quaternion(0.f,0.f,0.f,0.f);
		}
	}
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
	uint16 matId;			//材质id
	uint16 vstart;			// first vertex
	uint16 vcount;			// num vertices
	uint16 istart;			// first index
	uint16 icount;			// num indices
	bool		animated;
	char name[256];			// 子模型名称
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
	std::string name;	// 动画名称
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
	int parent;		// 父节点的id
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