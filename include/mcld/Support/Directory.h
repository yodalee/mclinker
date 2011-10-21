//===- Directory.h --------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_DIRECTORY_H
#define MCLD_DIRECTORY_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif
#include "llvm/Support/Allocator.h"
#include "mcld/Support/FileSystem.h"
#include "mcld/Support/Path.h"
#include "mcld/ADT/StringMap.h"
#include "mcld/ADT/TypeTraits.h"

#ifdef MCLD_DEBUG
#include<iostream>
#endif


namespace mcld {
namespace sys {
namespace fs {

class DirIterator;

/** \class Directory
 *  \brief A Directory object stores a Path object, a FileStatus object for
 *   non-symbolic link status, and a FileStatus object for symbolic link
 *   status. The FileStatus objects act as value caches.
 */
class Directory
{
friend StringMap<sys::fs::Path*>::iterator detail::bring_one_into_cache(DirIterator& pIter);
friend void detail::open_dir(Directory& pDir);
friend void detail::close_dir(Directory& pDir);
private:
  friend class DirIterator;
  typedef StringMap<sys::fs::Path*> PathCache;

public:
  typedef DirIterator iterator;

public:
  /// default constructor
  Directory();

  /// constructor - a directory whose path is pPath
  explicit Directory(const Path& pPath,
                     FileStatus st = FileStatus(),
                     FileStatus symlink_st = FileStatus());

  /// copy constructor
  /// when a copying construction happens, the cache is not copied.
  Directory(const Directory& pCopy);

  /// assignment
  /// When an assignment occurs, the cache is clear.
  Directory& operator=(const Directory& pCopy);

  /// destructor, inheritable.
  virtual ~Directory();

  /// Since we have default construtor, we must provide assign.
  void assign(const Path& pPath,
              FileStatus st = FileStatus(),
              FileStatus symlink_st = FileStatus());

  /// clear - clear the cache and close the directory handler
  void clear();

  bool isGood() const;

  /// path - the path of the directory
  const Path& path() const
  { return m_Path; }

  FileStatus status() const;
  FileStatus symlinkStatus() const;

  // -----  iterators  ----- //
  // While the iterators move, the direcotry is modified.
  // Thus, we only provide non-constant iterator.
  iterator begin();
  iterator end();

  PathCache& cache()
  { return m_Cache; }

protected:
  mcld::sys::fs::Path m_Path;
  mutable FileStatus m_FileStatus;
  mutable FileStatus m_SymLinkStatus;
  intptr_t m_Handler;
  // the cache of directory
  PathCache m_Cache;
};

/** \class DirIterator
 *  \brief A DirIterator object can traverse all entries in a Directory
 *
 *  DirIterator will open the directory and add entry into Directory::m_Cache
 *  DirIterator() is the end of a directory.
 *  If the end of the directory elements is reached, the iterator becomes
 *  equal to the end iterator value - DirIterator().
 *
 *  @see Directory
 */
class DirIterator
{
friend Directory::PathCache::iterator detail::bring_one_into_cache(DirIterator& pIter);
friend class Directory;
public:
  typedef Directory::PathCache            DirCache;

public:
  typedef Directory                       value_type;
  typedef ConstTraits<Directory>          const_traits;
  typedef NonConstTraits<Directory>       non_const_traits;
  typedef std::input_iterator_tag         iterator_category;
  typedef size_t                          size_type;
  typedef ptrdiff_t                       difference_type;

private:
  explicit DirIterator(Directory* pParent,
                       const DirCache::iterator& pIter);

public:
  // Since StringMapIterator has no default constructor, we also have none.
  DirIterator(const DirIterator &X);
  ~DirIterator();
  DirIterator& operator=(const DirIterator& pCopy);

  DirIterator& operator++();
  DirIterator operator++(int);

  Path* generic_path();

  Path* path();
  const Path* path() const;

  bool operator==(const DirIterator& y) const;
  bool operator!=(const DirIterator& y) const;

private:
  Directory* m_pParent; // get handler
  Directory::PathCache::iterator m_Idx;
};

} // namespace of fs
} // namespace of sys
} // namespace of mcld

#endif
