// natFileWin32.cc - Native part of File class for Win32.

/* Copyright (C) 1998, 1999, 2001  Red Hat, Inc.

   This file is part of libgcj.

This software is copyrighted work licensed under the terms of the
Libgcj License.  Please consult the file "LIBGCJ_LICENSE" for
details.  */

#include <config.h>

#include <stdio.h>
#include <string.h>

#include <windows.h>

#include <gcj/cni.h>
#include <jvm.h>
#include <java/io/File.h>
#include <java/io/IOException.h>
#include <java/util/Vector.h>
#include <java/lang/String.h>
#include <java/io/FilenameFilter.h>
#include <java/lang/System.h>

jboolean
java::io::File::_access (jint query)
{
  char buf[MAX_PATH];
  jsize total = JvGetStringUTFRegion (path, 0, path->length(), buf);
  // FIXME?
  buf[total] = '\0';

  JvAssert (query == READ || query == WRITE || query == EXISTS);

  // FIXME: Is it possible to differentiate between existing and reading?
  // If the file exists but cannot be read because of the secuirty attributes
  // on an NTFS disk this wont work (it reports it can be read but cant)
  // Could we use something from the security API?
  DWORD attributes = GetFileAttributes (buf);
  if ((query == EXISTS) || (query == READ))
    return (attributes == 0xffffffff) ? false : true;
  else
    return ((attributes != 0xffffffff) && ((attributes & FILE_ATTRIBUTE_READONLY) == 0)) ? true : false;
}

jboolean
java::io::File::_stat (jint query)
{
  char buf[MAX_PATH];
  jsize total = JvGetStringUTFRegion (path, 0, path->length(), buf);
  // FIXME?
  buf[total] = '\0';

  // FIXME: Need to handle ISHIDDEN query.

  JvAssert (query == DIRECTORY || query == ISFILE);

  DWORD attributes = GetFileAttributes (buf);
  if (attributes == 0xffffffff)
    return false;

  if (query == DIRECTORY)
    return attributes & FILE_ATTRIBUTE_DIRECTORY ? true : false;
  else
    return attributes & FILE_ATTRIBUTE_DIRECTORY ? false : true;
}

jlong
java::io::File::attr (jint query)
{
  char buf[MAX_PATH];
  jsize total = JvGetStringUTFRegion (path, 0, path->length(), buf);
  // FIXME?
  buf[total] = '\0';

  JvAssert (query == MODIFIED || query == LENGTH);

  WIN32_FILE_ATTRIBUTE_DATA info;
  if (! GetFileAttributesEx(buf, GetFileExInfoStandard, &info))
    return 0;

  if (query == LENGTH)
    return ((long long)info.nFileSizeHigh) << 32 | (unsigned long long)info.nFileSizeLow;
  else {
    // FIXME? This is somewhat compiler dependant (the LL constant suffix)
    // The file time as return by windows is the number of 100-nanosecond intervals since January 1, 1601
    return (((((long long)info.ftLastWriteTime.dwHighDateTime) << 32) | ((unsigned long long)info.ftLastWriteTime.dwLowDateTime)) - 116444736000000000LL) / 10000LL;
  }
}

jstring
java::io::File::getCanonicalPath (void)
{
  char buf[MAX_PATH], buf2[MAX_PATH];
  jsize total = JvGetStringUTFRegion (path, 0, path->length(), buf);
  // FIXME?
  buf[total] = '\0';

  LPTSTR unused;
  if(!GetFullPathName(buf, MAX_PATH, buf2, &unused))
    throw new IOException (JvNewStringLatin1 ("GetFullPathName failed"));

  // FIXME: what encoding to assume for file names?  This affects many
  // calls.
  return JvNewStringUTF(buf2);
}

jboolean
java::io::File::isAbsolute (void)
{
  if (path->charAt(0) == '/' || path->charAt(0) == '\\')
    return true;
  if (path->length() < 3)
    return false;
  // Hard-code A-Za-z because Windows (I think) can't use non-ASCII
  // letters as drive names.
  if ((path->charAt(0) < 'a' || path->charAt(0) > 'z')
      && (path->charAt(0) < 'A' || path->charAt(0) > 'Z'))
    return false;
  return (path->charAt(1) == ':'
	  && (path->charAt(2) == '/' || path->charAt(2) == '\\'));
}

jstringArray
java::io::File::performList (java::io::FilenameFilter *filter, 
			     java::io::FileFilter *fileFilter, 
			     java::lang::Class *result_type)
{
  char buf[MAX_PATH];
  jsize total = JvGetStringUTFRegion (path, 0, path->length(), buf);
  // FIXME?
  strcpy(&buf[total], "\\*.*");

  WIN32_FIND_DATA data;
  HANDLE handle = FindFirstFile (buf, &data);
  if (handle == INVALID_HANDLE_VALUE)
    return NULL;

  java::util::ArrayList *list = new java::util::ArrayList ();

  do
    {
      if (strcmp (data.cFileName, ".") && strcmp (data.cFileName, ".."))
        {
          jstring name = JvNewStringUTF (data.cFileName);

	  if (filter && ! filter->accept(this, name))
	    continue;

	  if (result_type == &java::io::File::class$)
            {
	      java::io::File *file = new java::io::File (this, name);
	      if (fileFilter && ! fileFilter->accept(file))
		continue;

	      list->add(file);
	    }
	  else
	    list->add(name);
	}
    }
  while (FindNextFile (handle, &data));

  if (GetLastError () != ERROR_NO_MORE_FILES)
    return NULL;

  FindClose (handle);

  jobjectArray ret = JvNewObjectArray (vec->size(), path->getClass(), NULL);
  vec->copyInto (ret);
  return reinterpret_cast<jstringArray> (ret);
}

jboolean
java::io::File::performMkdir (void)
{
  char buf[MAX_PATH];
  jsize total = JvGetStringUTFRegion(path, 0, path->length(), buf);
  // FIXME?
  buf[total] = '\0';

  return (CreateDirectory(buf, NULL)) ? true : false;
}

jboolean
java::io::File::performSetReadOnly (void)
{
  // PLEASE IMPLEMENT ME
  return false;
}

JArray< ::java::io::File *>*
java::io::File::performListRoots ()
{
  // PLEASE IMPLEMENT ME
  return NULL;
}

jboolean
java::io::File::performRenameTo (File *dest)
{
  char buf[MAX_PATH];
  jsize total = JvGetStringUTFRegion(path, 0, path->length(), buf);
  // FIXME?
  buf[total] = '\0';
  char buf2[MAX_PATH];
  total = JvGetStringUTFRegion(dest->path, 0, dest->path->length(), buf2);
  // FIXME?
  buf2[total] = '\0';

  return (MoveFile(buf, buf2)) ? true : false;
}

jboolean
java::io::File::performSetLastModified (jlong time)
{
  // PLEASE IMPLEMENT ME
  return false;
}

jboolean
java::io::File::performCreate (void)
{
  // PLEASE IMPLEMENT ME
  return false;
}

jboolean
java::io::File::performDelete ()
{
  char buf[MAX_PATH];
  jsize total = JvGetStringUTFRegion(path, 0, path->length(), buf);
  // FIXME?
  buf[total] = '\0';

  DWORD attributes = GetFileAttributes (buf);
  if (attributes == 0xffffffff)
    return false;

  if (attributes & FILE_ATTRIBUTE_DIRECTORY)
    return (RemoveDirectory (buf)) ? true : false;
  else
    return (DeleteFile (buf)) ? true : false;
}

void
java::io::File::init_native ()
{
  maxPathLen = MAX_PATH;
  caseSensitive = false;
}
