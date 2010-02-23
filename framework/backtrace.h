#include <QObject>
#include <QString>
#include <QStringList>

#define _HAVE_EXECINFO_H_ 1

#ifdef _HAVE_EXECINFO_H_
  #include <execinfo.h>
  #include <cxxabi.h>
#endif

/** Obtain a backtrace and return as a QStringList.
    This is not well-optimised, requires compilation with "-rdynamic" on linux
    and doesn't do a great job of demangling the symbol names. It is sufficient
    though to work out call trace. */
QStringList getBackTrace(int maxdepth=25)
{
    //now get the backtrace of the code at this point
    //(we can only do this if we have 'execinfo.h'
#ifdef _HAVE_EXECINFO_H_
    
    void *func_addresses[maxdepth];
    int nfuncs = backtrace(func_addresses, maxdepth);

    //now get the function names associated with these symbols. This should work for elf
    //binaries, though additional linker options may need to have been called 
    //(e.g. -rdynamic for GNU ld. See the glibc documentation for 'backtrace')
    char **symbols = backtrace_symbols(func_addresses, nfuncs);
   
    //save all of the function names onto the QStringList....
    //(note that none of this will work if we have run out of memory)
    QStringList ret;
    
    for (int i=0; i<nfuncs; i++)
    {
        
        int stat;
        char *demangled = abi::__cxa_demangle(symbols[i],0,0,&stat);
        
        if (demangled)
        {
            ret.append(QString::fromAscii(demangled));
            delete demangled;
        }
        else
            //append the raw symbol
            ret.append(QString::fromAscii(symbols[i]));
    }
    
    //we now need to release the memory of the symbols array. Since it was allocated using
    //malloc, we must release it using 'free'
    free(symbols);

    return ret;

#else
    return QStringList( QObject::tr(
                "Backtrace is not available without execinfo.h")
                      );
#endif

}

