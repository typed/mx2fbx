#ifndef __MemoryStream_H__
#define __MemoryStream_H__

#include <string>

/// �ڴ�������Ա������ʹ�÷����ο�Stream�е�ע�ͣ�
class MemoryStream
{
protected:
	uint	m_growBytes;		/// ���ڴ治��ʱ��ÿ�ε������ֽ���
	uint	m_pos;				/// �ڴ��дλ��
	uint	m_bufSize;			/// ��������С
	uint	m_fileSize;			/// �ļ���С�� С�ڵ��� m_bufSize ��
	uchar*	m_buffer;			/// ������
	bool	m_autoDelete;		/// �Ƿ�������ʱ�Զ�ɾ��������

	std::string m_path;

public:
	/// ����Զ��������ڴ棬�ڴ滹δ����
	MemoryStream(uint growBytes = 1024);

	/// ����ĳ���Ѿ�����Ļ������Ĳ��������growBytes��Ϊ0����ô���뱣֤buffer�Ƕ��ڴ������ջ�ڴ�
	MemoryStream(uchar* buffer, uint bufSize, uint growBytes = 0);

	/// ����һ����СΪsize�Ļ�����������ʱ���Զ�ɾ��
	MemoryStream(uint size, uint growBytes = 1024);

	virtual ~MemoryStream();

public:
	/**����ָ���Ļ�����,���growBytes��Ϊ0����ô���뱣֤buffer�Ƕ��ڴ������ջ�ڴ�
	 @param buffer ��������ַ
	 @param bufSize ��������С���ֽ�����
	 @param growBytes ���ڴ治��ʱ���������ֽ���
	 @note growBytesΪ0һ�����ڶԻ��������ж��������������д����
	 */
	void attach(uchar* buffer, uint bufSize, uint growBytes = 0);

	/// ��ָ���Ļ��������룬��attachƥ��ʹ��
	uchar* detach();

	/// ��û�������ַ
	uchar* getBuffer() const;

	/// ��ȡ�ڴ治��ʱ�������ֽ���
	uint getGrowBytes() const;

	/// ��ȡ��������С�����ڵ����ļ���С��
	uint getBufferSize() const;

	// Stream�ӿ�
public:
	/// ���Ƿ���true
	virtual bool open(const char* mode = "rb");

	/// ����������ݣ������ڲ�����Ļ��������ͷţ������Ҫ�ͷŵĻ���
	virtual bool close();

	bool readString(char* str, uint& length);
	/** ���ڴ����
	 @param buffer ���ڴ洢���ݵĻ�����
	 @param toRead Ҫ��ȡ���ֽ���
	 @return �ɹ�����true�����򷵻�false
	 @note ���ܵ�ʧ��ԭ��<br>
		��bufferΪNULL����toReadΪ0,����������ָ��Խ���ڴ���β�������нضϲ���ʱ������true
	 */
	virtual bool read(void* buffer, uint toRead);

	/** д�ڴ����
	 @param buffer Ҫд����ڴ��ַ
	 @param toWrite Ҫд����ֽ���
	 @return �ɹ�����true�����򷵻�false
	 @note ���ܵ�ʧ��ԭ��<br>
		��buffer=0����toWrite=0�����Ѿ�д����β�����ڴ治֧������ʱ
	 */
	virtual bool write(const void* buffer, uint toWrite);

	/** �ڴ��дָ�붨λ
	 @param offset ƫ�ƴ�С
	 @param from ����ƫ��λ�õĲο�λ��
	 @return �ɹ�����true�����򷵻�false
	 @note ʧ�ܵĿ���ԭ��<br>
		������Ч��ͨ��������λ��< 0��ͨ��������λ�ô����ļ�β�����ڴ治֧������
	 */
	virtual bool seek(int offset, uint from = SEEK_SET);

	/// �ڴ��дָ�붨λ���ڴ濪ʼ��
	virtual bool seekToBegin();

	/// �ڴ��дָ�붨λ���ڴ��β��
	virtual bool seekToEnd();

	/// �õ���ǰ��λ��
	virtual int getPosition() const;

	/// ���Ƿ���true
	virtual bool flush() const;

	/// ��ȡ�ڴ����Ĵ�С��С�ڵ����ڴ滺������С��
	virtual uint getLength() const;

	/** �����ڴ����ĳ���
	 @param newLen Ҫ�趨���ڴ������ȣ���С���ڴ�����ǰ��Сʱ���ȡ�ڴ���β�����ݣ�
	 @return �ɹ�����true�������µĳ��ȳ�����������С������������֧������ʱ����false
	 */
	virtual bool setLength(uint newLen);

	/// ���Ƿ���true
	virtual bool isExist() const;

	/// ���Ƿ���true
	virtual bool isOpen() const;

	/** ��������
	@param name ����
	@return ���ǳɹ�����true
	*/ 
	virtual bool setPath(const char* path);

	/// ���Ƿ���false
	virtual bool remove();

	/// ���Ƿ���true
	virtual bool rename(const char* newName);

	/** �����ڴ��ļ�����׼�ļ�
	 @param newName Ҫ������ڴ��ļ�������ʹ�þ���·����ʹ�����Ŀ¼���ܻᵼ������������Ŀ¼�����ڻ��Զ�������Ŀ¼����֧�ֵݹ飩
	 @return �ɹ�����true��������������������ڣ�����false
	 */
	virtual bool save(const char* newName);
	/// �ͷ�������
	virtual void release();

protected:
	virtual uchar* _alloc(uint bytes);
	virtual uchar* _realloc(uchar* ptr, uint bytes);
	virtual void _free(uchar* ptr);
	virtual bool growFile(uint newLen);
};

#endif // __MEMSTREAM_H__
