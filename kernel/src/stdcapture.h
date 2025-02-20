#ifdef _MSC_VER
#include <io.h>
#pragma warning(push)
#pragma warning(disable:4996) //The POSIX name for this item is deprecated
#define pipe _pipe
#else
#include <unistd.h>
#endif
#include <fcntl.h>
#include <stdio.h>

namespace ep {

class StdCapture
{
public:
  int GetOldFd() { return m_oldFd; }

  StdCapture(FILE *stdStream) : m_capturing(false)
  {
#ifdef _MSC_VER
    FILE *pFile;
    freopen_s(&pFile, "nul", "w", stdStream);
#endif

    m_stream = stdStream;
    m_fd = fileno(stdStream);
    setvbuf(m_stream, NULL, _IONBF, 0);

    beginCapture();
  }

  ~StdCapture()
  {
    endCapture();
  }

  bool beginCapture()
  {
    if (m_capturing)
      return false;

#if defined(_MSC_VER)
    int result = pipe(m_pipe, 65536, O_BINARY);
#else
    int result = pipe(m_pipe);
#endif // _MSC_VER

    if (result)
    {
      ep::Kernel::getInstance()->logWarning(0, "Failed to open pipe {0}\n", errno);
      return false;
    }

#if !defined(_MSC_VER)
   for (int i = 0; i < 2; ++i)
    {
      int flags =  fcntl(m_pipe[i], F_GETFL);
      fcntl(m_pipe[i], F_SETFL, flags | O_NONBLOCK);
    }
#endif

    m_oldFd = dup(m_fd);
    dup2(m_pipe[WRITE], m_fd);

    m_capturing = true;

#ifndef _MSC_VER
    close(m_pipe[WRITE]);
    m_pipe[WRITE] = -1;
#endif
    return true;
  }

  bool isCapturing()
  {
    return m_capturing;
  }

  void endCapture()
  {
    if (!m_capturing)
      return;

    dup2(m_oldFd, m_fd);

    close(m_oldFd);
    m_oldFd = -1;

    close(m_pipe[READ]);
    m_pipe[READ] = -1;

#ifdef _MSC_VER
    close(m_pipe[WRITE]);
    m_pipe[WRITE] = -1;
#endif

    m_capturing = false;
  }

  String getCapture()
  {
    const int bufSize = 1025;
    char buf[bufSize] = { 0 };
    int bytesRead;

    m_captured.clear();

    do
    {
      bytesRead = 0;
#ifdef _MSC_VER
      if (!eof(m_pipe[READ]))
        bytesRead = read(m_pipe[READ], buf, bufSize - 1);
#else
      bytesRead = read(m_pipe[READ], buf, bufSize - 1);
#endif
      if (bytesRead > 0)
      {
        buf[bytesRead] = 0;
        m_captured.append(String(buf, bytesRead));
      }
    } while (bytesRead == (bufSize - 1));

    ptrdiff_t bytesWritten = 0;
    if(m_captured.length)
      bytesWritten = (ptrdiff_t)write(GetOldFd(), m_captured.ptr, (int)m_captured.length);

    if (bytesWritten != (ptrdiff_t)m_captured.length)
      ep::Kernel::getInstance()->logWarning(0, "Not all bytes written when capture ended. Attempted {0} Actual {1}\n", m_captured.length, bytesWritten);

    return m_captured;
  }

private:
  enum PIPES { READ, WRITE };

  int m_pipe[2];
  int m_fd;
  int m_oldFd;
  FILE *m_stream;
  bool m_capturing;
  MutableString<1024> m_captured;
};

} // end namespace ep


#ifdef _MSC_VER
#pragma warning(pop)
#undef pipe
#endif // _MSC_VER
