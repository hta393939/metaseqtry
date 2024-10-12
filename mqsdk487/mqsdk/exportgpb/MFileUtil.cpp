#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <shlwapi.h>
#include <ShlObj.h>
#endif
#if __APPLE__
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <deque>
#include "osx/MStringUtil.h"
#endif
#include <stdlib.h>
#include "MFileUtil.h"
#include "MString.h"
#include "MAnsiString.h"


inline static bool isPathSeparator(wchar_t character)
{
	return (character == L'\\' || character == L'/');
}

inline static wchar_t createPathSeparator(const MString& path)
{
	for(const wchar_t *ptr = path.c_str() + path.length(); ptr > path.c_str(); ){
		ptr = path.prev(ptr);
		if(isPathSeparator(*ptr)){
			return *ptr;
		}
	}
	return MFILEPATH_DELIMITER_CHAR; // return default separator
}


//------------------------------------------------------------------
//  class MFileUtil
//------------------------------------------------------------------

// Check if a file exists
bool MFileUtil::fileExists(const MString& filename)
{
#ifdef WIN32
	return ::PathFileExistsW(filename.c_str()) && !::PathIsDirectoryW(filename.c_str());
#endif
#if __APPLE__
	struct stat st;
	int ret = stat(filename.toUtf8String().c_str(), &st);
	if(ret != 0) return false;
	if(!S_ISREG(st.st_mode)) return false;
	return true;
	
#endif
}

// Check if a destination of a path is a directory
bool MFileUtil::directoryExists(const MString& path)
{
#ifdef WIN32
	return ::PathIsDirectoryW(path.c_str()) ? true : false;
#endif
#if __APPLE__
	struct stat st;
	int ret = stat(path.toUtf8String().c_str(), &st);
	if(ret != 0) return false;
	if(!S_ISDIR(st.st_mode)) return false;
	return true;
#endif
}

// Check if a file is read only.
bool MFileUtil::isFileReadOnly(const MString& filename)
{
#ifdef WIN32
	DWORD attr = ::GetFileAttributes(filename);
	if(attr == 0xFFFFFFFF)
		return false; // file not found
	if(attr & FILE_ATTRIBUTE_READONLY)
		return true;
	return false;
#else
	int ret = access(filename.toUtf8String().c_str(), F_OK);
	if(ret != 0)
		return false; // file not found
	ret = access(filename.toUtf8String().c_str(), W_OK);
	return (ret != 0);
#endif
}

// Get a current directory.
MString MFileUtil::getCurrentDirectory()
{
#ifdef WIN32
	wchar_t path[MAX_PATH];
	DWORD len = ::GetCurrentDirectory(_countof(path), path);
	if(len == 0){
		return MString();
	}
	return MString(path);
#endif
#if __APPLE__
	char *buf = getcwd(nullptr, 0);
	if(buf != nullptr){
		MString ret = MString::fromUtf8String(buf);
		free(buf);
		return ret;
	}
	return MString();
#endif
}

// Set a current directory.
void MFileUtil::setCurrentDirectory(const MString& dir)
{
#ifdef WIN32
	::SetCurrentDirectory(dir.c_str());
#endif
#if __APPLE__
	chdir(dir.toUtf8String().c_str());
#endif
}

// Enum files in the directory
std::vector<MString> MFileUtil::enumFilesInDirectory(const MString& dir_path, const MString& filter, bool file_only, bool exclude_hidden)
{
	std::vector<MString> ret;

#ifdef WIN32
	WIN32_FIND_DATAW fd;
	HANDLE handle = ::FindFirstFileW(combinePath(dir_path, filter).c_str(), &fd);
	if(handle == INVALID_HANDLE_VALUE){
		return ret;
	}

	do {
		if((wcscmp(fd.cFileName, L".") == 0) || (wcscmp(fd.cFileName, L"..") == 0)){
			continue; // current or up directory is ignored
		}

		if(file_only && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			continue; // directory is ignored
		}
		if(exclude_hidden && (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)){
			continue; // hidden file or directory is ignored
		}

		ret.push_back(fd.cFileName);
	} while(::FindNextFileW(handle, &fd));
	::FindClose(handle);
#endif
#if __APPLE__
	DIR *dp;
	struct dirent *ep;
	dp = opendir(dir_path.toUtf8String().c_str());
	if(dp != nullptr){
		while((ep = readdir(dp)) != nullptr){
			MString fn = MString::fromUtf8String(ep->d_name);
			if(fn == L"." || fn == L"..") continue;
			if(filter.length() != 0 && filter != L"*"){
				if(!fn.isMatch(filter))
					continue;
			}
			MString fullpath = MFileUtil::combinePath(dir_path, fn);
			struct stat st;
			int sret = stat(fullpath.toUtf8String().c_str(), &st);
			if(sret != 0)
				continue;
			if(!S_ISREG(st.st_mode) && !S_ISDIR(st.st_mode)){
				continue; // not either file or directory
			}
			if(file_only && S_ISDIR(st.st_mode)){
				continue; // directory is ignored
			}
			if(exclude_hidden){
				if(MStringUtil::IsHiddenFile(fullpath, S_ISDIR(st.st_mode)))
					continue; // hidden file or directory is ignored
			}
			ret.push_back(fn); // OK!
		}
		closedir(dp);
	}
#endif

	return ret;
}

// Enum sub directories in the directory
std::vector<MString> MFileUtil::enumDirectoriesInDirectory(const MString& dir_path, bool exclude_hidden)
{
	std::vector<MString> ret;

#ifdef WIN32
	WIN32_FIND_DATAW fd;
	HANDLE handle = ::FindFirstFileW(combinePath(dir_path, L"*").c_str(), &fd);
	if(handle == INVALID_HANDLE_VALUE){
		return ret;
	}

	do {
		if((wcscmp(fd.cFileName, L".") == 0) || (wcscmp(fd.cFileName, L"..") == 0)){
			continue; // current or up directory is ignored
		}

		if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
			continue; // file is ignored
		}
		if(exclude_hidden && (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)){
			continue; // hidden directory is ignored
		}

		ret.push_back(fd.cFileName);
	} while(::FindNextFileW(handle, &fd));
	::FindClose(handle);
#endif
#if __APPLE__
	DIR *dp;
	struct dirent *ep;
	dp = opendir(dir_path.toUtf8String().c_str());
	if(dp != nullptr){
		while((ep = readdir(dp)) != nullptr){
			MString fn = MString::fromUtf8String(ep->d_name);
			if(fn == L"." || fn == L"..") continue;
			if(!MFileUtil::directoryExists(MFileUtil::combinePath(dir_path, fn))){
				continue;
			}
			MString fullpath = MFileUtil::combinePath(dir_path, fn);
			struct stat st;
			int sret = stat(fullpath.toUtf8String().c_str(), &st);
			if(sret != 0)
				continue;
			if(!S_ISDIR(st.st_mode)){
				continue; // file is ignored
			}
			if(exclude_hidden){
				if(MStringUtil::IsHiddenFile(fullpath, true))
					continue; // hidden directory is ignored
			}
			ret.push_back(fn); // OK!
		}
		closedir(dp);
	}
#endif

	return ret;
}

// Create a directory
bool MFileUtil::createDirectory(const MString& dst_path)
{
	if(directoryExists(dst_path)) return true;

#ifdef WIN32
#if 1
	std::vector<MString> up_dirs;
	up_dirs.push_back(dst_path);
	while(1){
		MString up_dir = getUpDirectory(up_dirs.back());
		if(up_dir.length() == 0 || directoryExists(up_dir)){
			break;
		}
		up_dirs.push_back(up_dir);
	}
	
	for(auto it = up_dirs.rbegin(); it != up_dirs.rend(); ++it){
		if(!::CreateDirectoryW((*it).c_str(), NULL)){
			return false;
		}
	}
	return true;
#else // SHCreateDirectoryExW() needs XP SP2 or later.
	int ret = ::SHCreateDirectoryExW(NULL, dst_path.c_str(), NULL);
	return ret == ERROR_SUCCESS;
#endif
#endif
#if __APPLE__
	BOOL ret = MStringUtil::CreateDirectory(dst_path);
	return ret;
#endif
}

// Copy a file
bool MFileUtil::copyFile(const MString& dst_file, const MString& src_file, bool can_overwrite)
{
#ifdef WIN32
	return ::CopyFileW(src_file.c_str(), dst_file.c_str(), can_overwrite ? FALSE : TRUE) ? true : false;
#endif
#if __APPLE__
	if(!can_overwrite){
		if(MFileUtil::fileExists(dst_file))
			return false;
	}
	BOOL ret = MStringUtil::CopyFile(dst_file, src_file);
	return ret;
#endif
}

// Copy a file with retrying if failed.
bool MFileUtil::copyFileWithRetry(const MString& dst_file, const MString& src_file, int retry_num, int interval_msec)
{
	for(int n = 0; n < retry_num; n++){
		if(copyFile(dst_file, src_file)){
			return true;
		}

		if(n == 0){
#ifdef WIN32
			DWORD err = GetLastError();
			if(err == ERROR_FILE_NOT_FOUND){
				return false; // Return immediately when the file is not found.
			}
#endif
		}
	}

	return false;
}

// Delete a file
bool MFileUtil::deleteFile(const MString& path)
{
#ifdef WIN32
	return ::DeleteFile(path.c_str()) ? true : false;
#endif
#if __APPLE__
	int ret = unlink(path.toUtf8String().c_str());
	return (ret == 0);
#endif
}

// Return a path that extension is changed
MString MFileUtil::changeExtension(const MString& src_path, const MString& extension)
{
	size_t len = src_path.length();

	for(const wchar_t *ptr = src_path.c_str() + len; ptr > src_path.c_str(); )
	{
		ptr = src_path.prev(ptr);
		if(*ptr == L'.') {
			return src_path.substring(0, ptr - src_path.c_str()) + extension;
		}
		if(isPathSeparator(*ptr)){
			return src_path + extension;
		}
	}
	
	// not find either period or \, so add extension
	return src_path + extension;
}

// Return an extension in the path
MString MFileUtil::extractExtension(const MString& src_path)
{
	size_t len = src_path.length();

	for(const wchar_t *ptr = src_path.c_str() + len; ptr > src_path.c_str(); )
	{
		ptr = src_path.prev(ptr);
		if(*ptr == L'.') {
			return src_path.substring(ptr + 1 - src_path.c_str());
		}
		if(isPathSeparator(*ptr))
			break;
	}

	return MString();
}

// Return a filename only (excluding an extension) in the path
MString MFileUtil::extractFileNameOnly(const MString& src_path)
{
	size_t len = src_path.length();
	const wchar_t *endptr = src_path.c_str() + len;
	const wchar_t *ptr;
	for(ptr = endptr; ptr > src_path.c_str(); )
	{
		ptr = src_path.prev(ptr);
		if(*ptr == L'.' && endptr == src_path.c_str() + len){
			endptr = ptr;
		}
		if(isPathSeparator(*ptr)){
			ptr = src_path.next(ptr);
			break;
		}
	}
	if(ptr >= endptr){
		return MString();
	}
	return src_path.substring(ptr - src_path.c_str(), endptr - ptr);
}

// Return a directory in the path
MString MFileUtil::extractDirectory(const MString& src_path)
{
	size_t len = src_path.length();

	for(const wchar_t *ptr = src_path.c_str() + len; ptr > src_path.c_str(); )
	{
		ptr = src_path.prev(ptr);
		if(isPathSeparator(*ptr)){
			MString ret = src_path.substring(0, ptr - src_path.c_str());
			if(ret.length() > 0 && !isPathSeparator(ret[ret.length()-1])){
				ret += createPathSeparator(ret);
			}
			return ret;
		}
	}
	return MString();
}

// Return a filename with an extension in the path
MString MFileUtil::extractFilenameAndExtension(const MString& src_path)
{
	size_t len = src_path.length();
	const wchar_t *endptr = src_path.c_str() + len;
	const wchar_t *ptr;
	for(ptr = endptr; ptr > src_path.c_str(); )
	{
		ptr = src_path.prev(ptr);
		if(isPathSeparator(*ptr)){
			ptr = src_path.next(ptr);
			break;
		}
	}
	if(ptr >= endptr){
		return MString();
	}
	return MString(ptr);
}

// Return a drive name in the path
MString MFileUtil::extractDrive(const MString& src_path)
{
	// extract "A:\"
	if( (src_path.length() >= 2) && 
		((src_path[0] >= L'A' && src_path[0] <= L'Z') || (src_path[0] >= L'a' && src_path[0] <= L'z')) && 
		src_path[1] == L':' )
	{
		return src_path.substring(0, 2) + MFILEPATH_DELIMITER_L;
	}

	// extract "\\server\"
	if( (src_path.length() >= 3) && 
		(isPathSeparator(src_path[0]) && src_path[0] == src_path[1]) )
	{
		for(const wchar_t *ptr = src_path.c_str() + 2; ptr < src_path.c_str() + src_path.length(); ){
			if(isPathSeparator(*ptr)){
				if(ptr == src_path.c_str() + 2){
					return MString();
				}
				return src_path.substring(0, ptr - src_path.c_str() + 1);
			}
			ptr = src_path.next(ptr);
		}
		return src_path + MFILEPATH_DELIMITER_L;
	}

	return MString();
}


// Return an up level directory
MString MFileUtil::getUpDirectory(const MString& src_path)
{
	size_t len = src_path.length();
	if(len == 0){
		return MString();
	}
	const wchar_t *endptr = src_path.c_str() + len;
	const wchar_t *ptr = src_path.prev(endptr);
	if(isPathSeparator(*ptr)){
		ptr = src_path.prev(ptr);
	}
	for(; ptr > src_path.c_str(); ptr = src_path.prev(ptr))
	{
		if(isPathSeparator(*ptr)) {
			// remove header "\\" for remote
			if(ptr == src_path.c_str() + 1 && isPathSeparator(ptr[-1])){
				return MString();
			}
			return src_path.substring(0, ptr - src_path.c_str() + 1);
		}
	}
	return MString();
}

// Combine two pathes
MString MFileUtil::combinePath(const MString& base_dir, const MString& cat_dir)
{
#ifdef WIN32
	wchar_t buf[MAX_PATH];
	::PathCombineW(buf, base_dir, cat_dir);
	return MString(buf);
#endif
#if __APPLE__
	MString ret;
	if(base_dir.length() > 0){
		ret += base_dir;
		if(base_dir[base_dir.length()-1] != L'\\' && base_dir[base_dir.length()-1] != L'/'){
			ret += L"/";
		}
	}
	ret += cat_dir;
	size_t p = ret.indexOf(L'\\');
	while(p != MString::kInvalid){
		ret[p] = L'/';
		p = ret.indexOf(L'\\', p+1);
	}
	return ret;
#endif
}

// Return a full path
MString MFileUtil::extractFullPath(const MString& src_path)
{
#ifdef WIN32
	wchar_t buf[MAX_PATH];
	_wfullpath(buf, src_path.c_str(), _countof(buf));
	return MString(buf);
#endif
#if __APPLE__
	MString path;
	if(src_path[0] == L'/')
		path = src_path;
	else
		path = MFileUtil::combinePath(getCurrentDirectory(), src_path);
	return MStringUtil::ConvertToFullPath(path);
#endif
}

MString MFileUtil::extractRelativePath(const MString& base_dir, const MString& src_path)
{
#ifdef WIN32
	const wchar_t *base_ptr = base_dir.c_str();

	// Trim the end path separator. (for WindowsXP)
	MString base_trim;
	if(base_dir.length() >= 2 && isPathSeparator(base_dir[base_dir.length()-1]) && !isPathSeparator(base_dir[base_dir.length()-2])){
		base_trim = base_dir.substring(0, base_dir.length()-1);
		base_ptr = base_trim.c_str();
	}

	bool is_src_dir = directoryExists(src_path);

	wchar_t buf[MAX_PATH];
	if(::PathRelativePathTo(buf, base_ptr, FILE_ATTRIBUTE_DIRECTORY, src_path.c_str(), is_src_dir ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL)){
		if(buf[0] == L'.' && buf[1] == L'\\'){
			return MString(buf+2);
		}else{
			return MString(buf);
		}
	}
	return MString();
#endif
#if __APPLE__
	std::deque<MString> base_level, src_level;
	MString dir = base_dir;
	if(dir.length() > 0 && !isPathSeparator(dir[dir.length()-1]))
		dir += L"/";
	while(dir.length() > 0){
		base_level.push_front(dir);
		dir = MFileUtil::getUpDirectory(dir);
	}
	dir = src_path;
	while(dir.length() > 0){
		src_level.push_front(dir);
		dir = MFileUtil::getUpDirectory(dir);
	}
	size_t level = 0;
	for(size_t n=0; n<base_level.size() && n<src_level.size(); n++){
		if(base_level[n].compare(src_level[n]) == 0){
			level++;
		}else{
			break;
		}
	}
	if(level > 0){
		MString ret;
		for(size_t l=level; l<base_level.size(); l++){
			ret += L"../";
		}
		ret += src_path.substring(base_level[level-1].length());
		return ret;
	}
	return src_path;
#endif
}

bool MFileUtil::isPathRelative(const MString& path)
{
#ifdef WIN32
	return ::PathIsRelative(path.c_str()) != FALSE;
#endif
#if __APPLE__
	if(path.length() > 0 && path[0] != L'/'){
		return true;
	}
	return false;
#endif
}

MString MFileUtil::getSystemDirectory(DirectoryType type)
{
#ifdef WIN32
	TCHAR path[MAX_PATH+32];
	switch(type){
	case kMyDocuments:
		if(::SHGetSpecialFolderPath(NULL, path, CSIDL_MYDOCUMENTS, FALSE)){
			return MString(path);
		}
		break;
	case kMyPictures:
		if(::SHGetSpecialFolderPath(NULL, path, CSIDL_MYPICTURES, FALSE)){
			return MString(path);
		}
		break;
	case kDesktop:
		if(::SHGetSpecialFolderPath(NULL, path, CSIDL_DESKTOP, FALSE)){
			return MString(path);
		}
		break;
	}
#endif
#if __APPLE__
	switch(type){
	case kMyDocuments:
		return MStringUtil::GetMyDocumentDir();
	case kMyPictures:
		return MStringUtil::GetMyPictureDir();
	case kDesktop:
		return MStringUtil::GetDesktopDir();
	}
#endif
	MLIBS_ASSERT(0);
	return MString();
}

