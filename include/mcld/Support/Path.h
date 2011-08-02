/*****************************************************************************
 *   The MCLinker Project, Copyright (C), 2011 -                             *
 *   Embedded and Web Computing Lab, National Taiwan University              *
 *   MediaTek, Inc.                                                          *
 *                                                                           *
 *   Chun-Hung Lu <chun-hung.lu@mediatek.com>                                *
 *   Luba Tang <lubatang@mediatek.com>                                       *
 *****************************************************************************
 * This file declares the llvm::sys::fs:: namespace. It follows TR2/boost    *
 * filesystem (v3), but modified to remove exception handling and the        *
 * path class.                                                               *
 ****************************************************************************/
#ifndef MCLD_PATH_H
#define MCLD_PATH_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif
#include <llvm/Support/raw_ostream.h>
#include <string>

namespace mcld {
namespace sys  {
namespace fs   {

/** \class Path
 *  \brief Path provides an abstraction for the path to a file or directory in
 *   the operating system's filesystem.
 *
 *  FIXME: current Path library only support UTF-8 chararcter set.
 *
 */
class Path
{
public:
#ifdef LLVM_ON_WIN32
  typedef wchar_t                            ValueType;
#else
  typedef char                               ValueType;
#endif
  typedef std::basic_string<ValueType>       StringType;

public:
  Path();
  Path(const ValueType* s );
  Path(const StringType &s );
  ~Path();

  // -----  assignments  ----- //
  template <class InputIterator>
  Path& assign(InputIterator begin, InputIterator end);

  Path& assign(const StringType &s);

  Path& assign(const ValueType* s, unsigned int length);

  //  -----  appends  ----- //
  template <class InputIterator>
  Path& append(InputIterator begin, InputIterator end);

  //  -----  observers  ----- //
  bool empty() const;
  const StringType &native() const { return m_PathName; }
  const ValueType* c_str() const   { return m_PathName.c_str(); }
  std::string string() const;

private:
  StringType::size_type m_append_separator_if_needed();
  void m_erase_redundant_separator(StringType &pPathName,
                                   StringType::size_type sep_pos) const;

private:
  StringType m_PathName;
};

//--------------------------------------------------------------------------------------//
//                              non-member functions                                    //
//--------------------------------------------------------------------------------------//
/// @param value a character
/// @result true if \a value is a path separator character on the host OS
bool is_separator(char value);
bool exists(const Path &pPath);
bool is_directory(const Path &pPath);
std::ostream &operator<<(std::ostream& pOS, const Path& pPath);
std::istream &operator>>(std::istream& pOS, const Path& pPath);
llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const Path &pPath);


//--------------------------------------------------------------------------------------//
//                     class path member template implementation                        //
//--------------------------------------------------------------------------------------//
template <class InputIterator>
Path& Path::assign(InputIterator begin, InputIterator end)
{
  m_PathName.clear();
  if (begin != end)
    m_PathName.append<InputIterator>(begin, end);
  return *this;
}

template <class InputIterator>
Path& Path::append(InputIterator begin, InputIterator end)
{
  if (begin == end)
    return *this;
  StringType::size_type sep_pos(m_append_separator_if_needed());
  m_PathName.append<InputIterator>(begin, end);
  if (sep_pos)
    m_erase_redundant_separator(m_PathName, sep_pos);
  return *this;
}

} // namespace of fs
} // namespace of sys
} // namespace of mcld

#endif

