#ifndef __DataChunk_H__
#define __DataChunk_H__

#include <vector>
#include "MemoryStream.h"

#define CHUNK_MIN_SIZE	8

class DataChunk
{
public:
	struct stChunk
	{
		friend class DataChunk;
	public:
		uint GetSize();
	public:
		stChunk(uint ui32Type);
		stChunk(uint ui32Type,uint ui32ChunkSize,uint ui32DataSize,void *pData,stChunk *pChild,stChunk *pSibling);
		~stChunk();
	public:
		uint		m_ui32Type;	//����

		// Helper Member,not saved in DataChunk-based files.
		uint		m_ui32ChunkSize;

		uint		m_ui32DataSize;
		void		*m_pData;	//����

	private:
		MemoryStream	m_data;
	public:
		stChunk	*m_pSibling;
		stChunk	*m_pChild;
	};
public:
	DataChunk();
	~DataChunk();
public:
	/**
	public interface that can deal with chunks.
	*/

	// read DataChunk data from DataStream,DataStream can be MemoryDataStream or FileDataStream
	void SerializeIn(MemoryStream* data,uint size);

	/** read DataChunk data from file
	@param bLoadAll true for load the Data of the DataChunk,false for only build the DataChunk structure.
	*/
	void SerializeIn(char szFileName[],bool bLoadAll = true);

	/** write DataChunk data to file
	@param szFileName the file to write to
	@remarks if szFileName is existed,the file will be overwritten.
	*/
	void save(const char* szFileName);
		
	/** write DataChunk data to Stream
	@param pStream the stream to write to
	*/
	void save(MemoryStream* pStream);

	/** destroy the DataChunk tree,this function is called by destructor automaticly.
	*/
	void Close();

private:
	/** fill all DataChunk's m_ui32ChunkSize field,then the DataChunk tree is perfectly represented.
	*/
	uint BuildChunkSize(stChunk* pChunk);
	stChunk* SerializeIn_Helper(MemoryStream* data, uint ui32FPBegin, uint ui32FPEnd, stChunk* pParent, stChunk* pBrother);
	typedef void (DataChunk::*pRecursiveFunc)(stChunk *,void*);
	void	RecursiveFunc(pRecursiveFunc pFunc, stChunk* pChunk, void*);
	stChunk* SerializeIn_Helper(FILE* fp, uint ui32FPBegin, uint ui32FPEnd, bool bLoadAll, stChunk* pParent, stChunk* pBrother);
	void	Close_Helper(stChunk* pChunk, void* p);
	void	SerializeOut_Helper(FILE* fp, stChunk* p);
	void	SerializeOut_Helper(MemoryStream* pStream, stChunk* p);
private:
	stChunk	*m_pChunk;
protected:
	/** operate DataChunk via strID
	@remarks base class just return,derived class should deal with this.
	*/
	virtual void operateChunk(uint ui32ID, MemoryStream* pData,uint size);
	//-------------------------------------------------------------------------------------------------------
public:
	//Read

	/**��ʼ��ȡһ��Chunk��ͨ����
	@param pData ����ָ��
	@return ����һ��stChunk
	*/
	stChunk* beginChunk(MemoryStream* pData, bool bWowFormat = false);

	/**��ʼ��ȡһ��Chunk��ͨ���ڴ�ָ��
	@param pData ����ָ��
	@param size �ڴ��С
	@return ����һ��stChunk
	*/
	stChunk* beginChunk(uchar* pData, uint size, bool bWowFormat = false);


	/**��ȡ��һ��Chunk��ͨ����
	@param pData ����ָ��
	@return ����һ��stChunk
	*/
	stChunk* nextChunk(MemoryStream* pData, bool bWowFormat = false);

	/**��ȡ��һ��Chunk
	@return ����һ��stChunk
	*/
	stChunk* nextChunk(bool bWowFormat = false);


	/**��õ�ǰ��ָ�������еľ���ƫ����
	@return ƫ����
	@remarks ����һЩ����
	*/
	uchar* getAbsoluteDataPointer();

	//Write

	/**��ʼ����һ��Chunk
	@param ui32Type Chunk���� 'MVER'
	@param pDataStream ����һ����������д��
	@return ����һ��Chunk
	@remarks
	*/
	stChunk* beginChunk(uint ui32Type, MemoryStream** pDataStream);

	/**����дһ��Chunk��ȷ��beginChunkҪ��Ӧһ��endChunk
	*/
	void endChunk();

	/**��õ�ǰ����ָ��������ļ��е�ƫ��ֵ
	@return ƫ��ֵ
	@remarks
	*/
	uint getOffset();
private:
	stChunk* m_pCurrentChunk;
	stChunk* readChunk(MemoryStream* pData, bool bWowFormat);
	MemoryStream m_data;
	//Write
	std::vector<stChunk*> m_vChunks;
	uint m_ui32Offset;
};

#endif