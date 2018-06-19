//# RegularFileIO.cc: Class for IO on a regular file
//# Copyright (C) 1996,1997,1998,1999,2001,2002
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#include <casacore/casa/aips.h>
#include <casacore/casa/IO/LargeIOFuncDef.h>
#include <casacore/casa/IO/RegularFileIO.h>
#include <casacore/casa/BasicSL/String.h>
#include <casacore/casa/Utilities/Assert.h>
#include <casacore/casa/Exceptions/Error.h>
#include <fcntl.h>
#include <errno.h>                    // needed for errno
#include <casacore/casa/string.h>               // needed for strerror

/*Headers for parallel read*/
#include<iostream>
#include <fstream>
#include <omp.h>
#include <string>
#include<cstdlib>
#include <boost/algorithm/string.hpp>

/* Required for mmap*/
#include <sys/mman.h>
#include <fcntl.h>



namespace casacore { //# NAMESPACE CASACORE - BEGIN



RegularFileIO::RegularFileIO (const RegularFile& regularFile,
                              ByteIO::OpenOption option,
                              uInt bufferSize)
: itsOption      (option),
  itsRegularFile (regularFile)
{
    int file = openCreate (regularFile, option);
    attach (file, (bufferSize == 0 ? 16384 : bufferSize));
    // If appending, set the stream offset to the file length.
    if (option == ByteIO::Append) {
        seek (length());
    }
}

RegularFileIO::~RegularFileIO()
{
    detach (True);
    if (itsOption == ByteIO::Scratch  ||  itsOption == ByteIO::Delete) {
	itsRegularFile.remove();
    }
}

std::string RegularFileIO::replace(std::string inp)
{
boost::replace_all(inp,"/","_");
return inp;
}

long RegularFileIO::get_file_size(std::string filename)
{
        std::ifstream In;
        In.open(filename.c_str());
        In.seekg(0,In.end);
        return In.tellg();

}

void RegularFileIO::threadedRead(std::string filename,char *buff,int threads,int fd)
{
        long file_size,priv_size,priv_fseek_start;//Size : total file size, priv* variables are thread specific
        int threadId,tot_thread; //tot_thread : total threads participating
        std::ifstream *inFile,In;
        std::string newFilePath=replace(filename);

        omp_set_num_threads(threads); //How many threads?
        inFile=new std::ifstream[threads];

        //Is there a better way to compute the file size?
        In.open(filename.c_str());
        In.seekg(0,In.end);
        file_size=In.tellg();
        fd=shm_open(newFilePath.c_str(),O_CREAT |O_RDWR,0777);

        buff=(char*)mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, fd, 0);
        ftruncate(fd,file_size);

        #pragma omp parallel private(priv_size,threadId,priv_fseek_start,tot_thread)
        {
                tot_thread=omp_get_num_threads();
                threadId=omp_get_thread_num();
                priv_size=file_size/tot_thread+1; //indices are aligned for parallel read
                priv_fseek_start=(threadId*file_size/tot_thread)+1;
                if(!threadId){priv_fseek_start-=-1;}
                inFile[threadId].open(filename.c_str());
                inFile[threadId].seekg(priv_fseek_start,inFile[threadId].beg);
                inFile[threadId].read(&buff[priv_fseek_start],priv_size);
                inFile[threadId].close();
        }

}

int RegularFileIO::omp_file_dump(std::string filename,char *buff,int threads)
{
        int fd;
        std::string newFilePath=replace(filename);
        fd=shm_open(newFilePath.c_str(),O_CREAT |O_RDWR,0777);
        //boost::thread(threadedRead,filename,buff,threads,fd);
        threadedRead(filename,buff,threads,fd);
        //boost::this_thread::sleep(boost::posix_time::seconds(30) );
        return fd;
}

void RegularFileIO::insertHash(std::string str,int myval,charAllocator chralloc,shmHashMap *map)
{
        shmKey mystr(chralloc);
        mystr=str.c_str();
        shmPair K(mystr,myval);
        map->insert(K);
}

int RegularFileIO::shmInsert(std::string db,std::string table,std::string filePath,int value)
{

    std::string newFilePath=replace(filePath);

    ipc::managed_shared_memory shm(ipc::open_or_create,db.c_str(),100000);
    int returnVal=0;
    charAllocator charalloc(shm.get_segment_manager());
    shmKey path(charalloc);
    path=newFilePath.c_str();

    char *buff;

    shmHashMap *readMap=shm.find<shmHashMap>(table.c_str()).first;
    if(readMap==0)
    {
       readMap=shm.construct<shmHashMap>(table.c_str())(300,boost::hash<shmKey>(),
                std::equal_to<shmKey>(), shm.get_allocator<shmPair>());
    }
    shmHashMap::iterator it=readMap->find(path);

    if(it==readMap->end())
    {
        returnVal=omp_file_dump(filePath,buff,28);
        insertHash(newFilePath,returnVal,charalloc,readMap);
        #ifdef PARIO_DEBUG
        std::cout<<"not found. fd"<<returnVal<<" "<<std::endl;
        #endif
   }
   else
   {
        //readMap->at(path)=value; 
        #ifdef PARIO_DEBUG
        std::cout<<"record:"<<readMap->at(path)<<std::endl;
        #endif
        //returnVal=readMap->at(path);
        returnVal=open((std::string("/dev/shm/")+newFilePath).c_str(),O_RDONLY);
  }

   return returnVal;


}




void RegularFileIO::removeShm(std::string inp)
{
        ipc::shared_memory_object::remove(inp.c_str());
}







int RegularFileIO::openCreate (const RegularFile& file,
                               ByteIO::OpenOption option)
{
    const String& name = file.path().expandedName();
    Bool create = False;
    Int stropt;
    switch (option) {
    case ByteIO::Old:
	stropt = O_RDONLY;
	break;
    case ByteIO::NewNoReplace:
	if (file.exists()) {
	    throw (AipsError ("RegularFileIO: new file " + name +
			      " already exists"));
	}
    case ByteIO::New:
    case ByteIO::Scratch:
        create = True;
	stropt = O_RDWR | O_CREAT | O_TRUNC;
	break;
    case ByteIO::Append:
	stropt = O_RDWR;
	break;
    case ByteIO::Update:
    case ByteIO::Delete:
	stropt = O_RDWR;
	break;
    default:
	throw (AipsError ("RegularFileIO: unknown open option"));
    }
    // Open the file.
    int fd;
    if (create) {
      fd = trace3OPEN ((char*)name.chars(), stropt, 0666);
    } else {
      fd = trace2OPEN ((char*)name.chars(), stropt);
     #if defined(PARIO) || defined(PARIO_DEBUG)
     if(get_file_size(name.chars())/1E9>4)
     {
	char *buff;
	close(fd);
        fd=shmInsert("wsclean","parallel_read",std::string(name.chars()),1);
	#ifdef PARIO_DEBUG
	std::cout<<"Current fd:"<<fd<<std::endl;
	#endif
        //std::cout<<name.chars()<<" fd:"<<fd<<"  size:"<<
	//	get_file_size(name.chars())/1E9<<endl;
    }
    #endif
  
    }
    if (fd < 0) {
	throw (AipsError ("RegularFileIO: error in open or create of file " +
			  name + ": " + strerror(errno)));
    }
    return fd;
}

void RegularFileIO::reopenRW()
{
    if (isWritable()) {
	return;
    }
    // First try if the file can be opened as read/write.
    const String& name = itsRegularFile.path().expandedName();
    int file = trace2OPEN ((char *)name.chars(), O_RDWR);
    if (file < 0) {
	throw (AipsError ("RegularFileIO::reopenRW "
			  "not possible for file " +
			  name + ": " + strerror(errno)));
    }
    uInt bufsize = bufferSize();
    detach (True);
    attach (file, bufsize);
    // It can be reopened, so close and reopen.
    itsOption = ByteIO::Update;
}


String RegularFileIO::fileName() const
{
    return itsRegularFile.path().expandedName();
}


} //# NAMESPACE CASACORE - END

