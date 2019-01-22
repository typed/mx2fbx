#include "stdafx.h"
#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>
#include "FileStream.h"

void FileStream::release()
{
	delete this;
}

FileStream::FileStream()
	: m_stream(0)
{
}

FileStream::FileStream(const char* path)
	: m_path(path)
	, m_stream(0)
{
}

FileStream::~FileStream()
{
	close();
}

bool FileStream::open(const char* mode)
{
	if (isOpen())
	{
		return false;
	}
	fopen_s(&m_stream, m_path.c_str(),mode);
	if (m_stream == NULL)
	{
		return false;
	}

	return true;
}

bool FileStream::close()
{
	if (m_stream != NULL)
	{
		if (fclose(m_stream) < 0) // EOS error?
		{
			return false;
		}

		m_stream = NULL;
	}

	return true;
}

bool FileStream::readString(char* str, uint& length)
{
	if (str == NULL || length == 0)
		return true;
	if (feof(m_stream))
		return false;

	char c;
	uint index = 0;
	while(fread(&c,1,sizeof(c),m_stream))
	{
		if(c == '\r')
		{
			char c;
			if(fread(&c,1,sizeof(c),m_stream))
			{
				if(c != '\n')
				{
					fseek(m_stream,-1,SEEK_CUR);
				}

				break;
			}
			else
			{
				break;
			}
		}

		str[index++] = c;
		if(index >= length - 1)break;
	}
	str[index] = 0;
	length = index;
	return true;
}

bool FileStream::read(void* buffer, uint toRead)
{
	if (buffer == NULL || toRead == 0) // avoid win32 null read
		return true;

	size_t size = fread(buffer, 1, toRead, m_stream);
	if (!size)
		return false;

	if (toRead != size) // eof error or read error(bad file, access)
	{
		if (!feof(m_stream))
		{
			return false;
		}
	}

	return true;
}

bool FileStream::write(const void* buffer, uint toWrite)
{

	if (buffer == NULL || toWrite == 0) // avoid win32 null write
		return true;

	if (toWrite != fwrite(buffer, 1, toWrite, m_stream))
	{
		return false;
	}
	return true;
}

bool FileStream::seek(int offset, uint from)
{
	return (fseek(m_stream, offset, from) == 0);
}

bool FileStream::seekToBegin()
{
	return seek(0, SEEK_SET);
}

bool FileStream::seekToEnd()
{
	return seek(0, SEEK_END);
}

bool FileStream::flush() const
{
	return fflush(m_stream) == 0;
}

uint FileStream::getLength() const
{
	if (isOpen())
	{
		return _filelength(_fileno(m_stream));
	}

	if (m_path.empty())
		return 0;

	struct _stat statbuf;
	_stat(m_path.c_str(), &statbuf);
	return statbuf.st_size;
}

bool FileStream::setLength(uint newLen)
{
	return _chsize(_fileno(m_stream), newLen) == 0;
}

int FileStream::getPosition() const
{
	
	int pos = -1;
	if ((pos = ftell(m_stream)) != -1) // success
		return pos;
	// failed
	return pos;
}


//////////////////////////// other /////////////////////////////////////
bool FileStream::isExist() const
{
	if (isOpen())
		return true;

	if (m_path.empty())
		return false;

	return _access(m_path.c_str(), 0) != -1;
}

bool FileStream::isOpen() const
{
	return m_stream != 0;
}

bool FileStream::remove()
{
	close();
	return ::remove(m_path.c_str()) == 0;
}

bool FileStream::rename(const char* newName)
{
	
	bool isopen = isOpen();
	int oldPos = 0;
	if (isopen)
		oldPos = getPosition();

	close();
	
	if (::rename(m_path.c_str(), newName) != 0)
		return false;
	
	m_path = newName;
	if (isopen)
	{
		return (open() && seek(oldPos));
	}

	return true;
}

bool FileStream::save(const char* newName)
{
	
	if (newName == NULL)
		return flush();

	int oldPos = 0;
	uint size = 0;
	uchar* buffer = 0;

	FileStream fs(newName);
	bool isopen = isOpen();
	if (!fs.open("wb"))
		goto IOErr;

	if (isopen)
	{
		flush();
		oldPos = getPosition();
	}
	else
	{
		if (!open())
			goto IOErr;
	}

	size = getLength();
	buffer = 0;
	if (size > 0)
	{
		buffer = new uchar[size];
		if (!seekToBegin() || !read(buffer, size) || !fs.write(buffer, size))
			goto IOErr;
	}
	fs.close();

	if (isopen)
		seek(oldPos);
	else
		close();

	delete[] buffer;
	return true;

IOErr:
	if (!isopen && isOpen())
		close();
	fs.close();
	delete[] buffer;
	return false;
}

const char * FileStream::getRelativePath( void ) const
{
	const char* pWorkDir = "";
	const char * pRelativePath = strstr(m_path.c_str(), pWorkDir);
	if (NULL != pRelativePath)
	{
		return pRelativePath + strlen(pWorkDir) + 1; // \Ö®ºó
	}
	return m_path.c_str();
}
