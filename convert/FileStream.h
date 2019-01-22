#ifndef __FILESTREAM_H__
#define __FILESTREAM_H__

#include <string>

/// ��׼�ļ�������������ʹ�òο�Stream�Ķ�Ӧ������
class FileStream
{
protected:
	FILE* m_stream;		/// �ļ������
	std::string m_path;
public:
	FileStream();
	FileStream(const char* path);
	virtual ~FileStream();

public:
	/// ����ļ������
	FILE* getHandle() const		{ return m_stream; }

	/// �ļ���ת�����ļ����
	operator FILE*() const		{ return m_stream; }

	// Stream�ӿ�
public:
	/** ���ļ���(Ĭ����ֻ������������ʽ��)
	 @param mode �ļ���ģʽ
	 @return �ɹ�����true�����򷵻�false
	 @note �й��ļ���ģʽ����Ϣ��<br>

	- "r"	��    ,�ļ���������ʧ��<br>
	- "w"	д    ,�ļ��������򴴽�,�������������<br>
	- "a"	���  ,�ļ��������򴴽�<br>
	- "r+"	��д  ,�ļ���������ʧ��<br>
	- "w+"	��д  ,�ļ��������򴴽�,�������������<br>
	- "a+"	�����,�ļ��������򴴽�<br>
	- "t"	�ı���ʽ<br>
	- "b"	�����Ʒ�ʽ<br>
	 */
	virtual bool open(const char* mode = "rb");

	/// �ر��ļ���
	virtual bool close();

	virtual bool readString(char* str, uint& length);
	/** ���ļ����ж�ȡ���ݣ����ڰ��ļ��е��ļ�����ܾ�����Խ��Ķ�д��
	 @param buffer ��������ַ
	 @param toRead Ҫ��ȡ���ֽڴ�С
	 @return �ɹ�����true�����򷵻�false
	 @note ����д�ļ�ʱ,����EOF��������Ҳ����true����Ϊ��Ӱ�����ݵ���ȷ�ԣ�<br>
		ֻ�е�bad file,access֮��Ĵ���ŷ���false
	 */
	virtual bool read(void* buffer, uint toRead);

	/** ���ļ�����д�����ݣ����ڰ��ļ��е��ļ�����ܾ�����Խ��Ķ�д��
	 @param buffer ��������ַ
	 @param toWrite Ҫд����ֽڴ�С
	 @return �ɹ�����true�����򷵻�false
	 @note ����д�ļ�ʱ,����EOF��������Ҳ����true����Ϊ��Ӱ�����ݵ���ȷ�ԣ�<br>
		ֻ�е�bad file,access֮��Ĵ���ŷ���false
	 */
	virtual bool write(const void* bufer, uint toWrite);

	/// �÷�ͬStream
	virtual bool seek(int offset, uint from = SEEK_SET);

	/// �÷�ͬStream
	virtual bool seekToBegin();

	/// �÷�ͬStream
	virtual bool seekToEnd();

	/// �÷�ͬStream
	virtual int getPosition() const;

	/// �÷�ͬStream
	virtual bool flush() const;

	/// �÷�ͬStream
	virtual uint getLength() const;

	/// �÷�ͬStream
	virtual bool setLength(uint newLen);

	virtual const char * getRelativePath(void)  const ;

	/// �÷�ͬStream
	virtual bool isExist() const;

	/// �÷�ͬStream
	virtual bool isOpen() const;

	/// ɾ���ļ���
	virtual bool remove();

	/** �������ļ���
	 @param newName �µ��ļ�������
	 @return �ɹ�����true�����򷵻�false
	 @note ��ǰ���ļ�����Ȼ�������������ļ�����״̬���䣬���ļ���δ�رգ���ô����Ҳû�йرգ��ļ�ָ���ȡλ��Ҳ���䡣
	 */ 
	virtual bool rename(const char* newName);

	/** �ļ����洢Ϊ�ļ�
	 @param newName Ҫ������ļ���
	 @return �ɹ�����true�����򷵻�false
	 @note ��newNameΪNULLʱ���ú����൱��flush�������൱�ڽ��ļ�������
		������copyһ�����Ϊһ���ļ���save��ǰ���ļ���״̬���䡣
	 */
	virtual bool save(const char* newName);
	/// �ͷ�������
	virtual void release();
};

#endif // __FILESTREAM_H__
