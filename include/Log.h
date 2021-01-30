#ifndef __LOG_H__
#define __LOG_H__
#include <iostream>
#include <fstream>
#include <string>

#ifndef __FILENAME__
#define __FILENAME__ __FILE__
#endif


// log日志系统

#define LOG(level)\
if(level>yn::Log::get().getLevel()) ;\
else yn::Log::get().getStream()<<'['<<__FILE__<<":"<<std::dec<<__LINE__<<']'


#define LOG_CPU \
if (yn::CpuTrace != yn::Log::get().getLevel()) ; \
else yn::Log::get().getCpuTraceStream()

namespace yn
{
    enum LogLevel
    {
        None,
        Error,
        Warn,
        Info,
        Debug,
        CpuTrace
    };
    class Log
    {
    public:
        ~Log();
        void setLogStream(std::ostream &stream);
        void setTraceStream(std::ostream &stream);
        Log &setLevel(LogLevel level);
        LogLevel getLevel();

        std::ostream &getStream();
        std::ostream &getCpuTraceStream();

        static Log &get();

    private:
        LogLevel m_logLevel;
        std::ostream *m_logStream;
        std::ostream *m_CpuTrace;
    };

    //Courtesy of http://wordaligned.org/articles/cpp-streambufs#toctee-streams
    class TeeBuf : public std::streambuf
    {
    public:
        // Construct a streambuf which tees output to both input
        // streambufs.
        TeeBuf(std::streambuf *sb1, std::streambuf *sb2);

    private:
        // This tee buffer has no buffer. So every character "overflows"
        // and can be put directly into the teed buffers.
        virtual int overflow(int c);
        // Sync both teed buffers.
        virtual int sync();

    private:
        std::streambuf *m_sb1;
        std::streambuf *m_sb2;
    };

    class TeeStream : public std::ostream
    {
    public:
        // Construct an ostream which tees output to the supplied
        // ostreams.
        TeeStream(std::ostream &o1, std::ostream &o2);

    private:
        TeeBuf m_tbuf;
    };
} // namespace yn
#endif // __LOG_H__