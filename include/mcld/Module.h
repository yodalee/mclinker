//===- Module.h -----------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Module contains the intermediate representation (LDIR) of MCLinker.
//
//===----------------------------------------------------------------------===//
#ifndef MCLD_MODULE_H
#define MCLD_MODULE_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif

#include <vector>
#include <string>

#include <llvm/ADT/ilist.h>

#include <mcld/ADT/HashTable.h>
#include <mcld/ADT/HashEntry.h>
#include <mcld/Support/GCFactoryListTraits.h>
#include <mcld/Fragment/Fragment.h>
#include <mcld/LD/NamePool.h>
#include <mcld/MC/SymbolCategory.h>
#include <mcld/MC/MCLDInput.h>
#include <mcld/MC/InputTree.h>

namespace mcld {

class LDSection;
class LDSymbol;
class RelocationData;

/** \class Module
 *  \brief Module provides the intermediate representation for linking.
 */
class Module
{
public:
  typedef std::vector<Input*> ObjectList;
  typedef ObjectList::iterator obj_iterator;
  typedef ObjectList::const_iterator const_obj_iterator;

  typedef std::vector<Input*> LibraryList;
  typedef LibraryList::iterator lib_iterator;
  typedef LibraryList::const_iterator const_lib_iterator;

  typedef InputTree::iterator input_iterator;
  typedef InputTree::const_iterator const_input_iterator;

  typedef std::vector<LDSection*> SectionTable;
  typedef SectionTable::iterator iterator;
  typedef SectionTable::const_iterator const_iterator;

  typedef SymbolCategory SymbolTable;
  typedef SymbolTable::iterator sym_iterator;
  typedef SymbolTable::const_iterator const_sym_iterator;

  typedef std::vector<RelocationData*> RelocationDataTable;
  typedef RelocationDataTable::iterator reloc_data_iterator;
  typedef RelocationDataTable::const_iterator const_reloc_data_iterator;

public:
  Module();

  Module(const std::string& pName);

  ~Module();

  // -----  name  ----- //
  const std::string& name() const { return m_Name; }

  void setName(const std::string& pName) { m_Name = pName; }

  // -----  link-in objects ----- //
  const ObjectList& getObjectList() const { return m_ObjectList; }
  ObjectList&       getObjectList()       { return m_ObjectList; }

  const_obj_iterator obj_begin() const { return m_ObjectList.begin(); }
  obj_iterator       obj_begin()       { return m_ObjectList.begin(); }
  const_obj_iterator obj_end  () const { return m_ObjectList.end();   }
  obj_iterator       obj_end  ()       { return m_ObjectList.end();   }

  // -----  link-in libraries  ----- //
  const LibraryList& getLibraryList() const { return m_LibraryList; }
  LibraryList&       getLibraryList()       { return m_LibraryList; }

  const_lib_iterator lib_begin() const { return m_LibraryList.begin(); }
  lib_iterator       lib_begin()       { return m_LibraryList.begin(); }
  const_lib_iterator lib_end  () const { return m_LibraryList.end();   }
  lib_iterator       lib_end  ()       { return m_LibraryList.end();   }

  // -----  link-in inputs  ----- //
  const InputTree& getInputTree() const { return m_MainTree; }
  InputTree&       getInputTree()       { return m_MainTree; }

  const_input_iterator input_begin() const { return m_MainTree.begin(); }
  input_iterator       input_begin()       { return m_MainTree.begin(); }
  const_input_iterator input_end  () const { return m_MainTree.end();   }
  input_iterator       input_end  ()       { return m_MainTree.end();   }

/// @}
/// @name Section Accessors
/// @{

  // -----  sections  ----- //
  const SectionTable& getSectionTable() const { return m_SectionTable; }
  SectionTable&       getSectionTable()       { return m_SectionTable; }

  iterator         begin()       { return m_SectionTable.begin(); }
  const_iterator   begin() const { return m_SectionTable.begin(); }
  iterator         end  ()       { return m_SectionTable.end();   }
  const_iterator   end  () const { return m_SectionTable.end();   }
  LDSection*       front()       { return m_SectionTable.front(); }
  const LDSection* front() const { return m_SectionTable.front(); }
  LDSection*       back ()       { return m_SectionTable.back();  }
  const LDSection* back () const { return m_SectionTable.back();  }
  size_t           size () const { return m_SectionTable.size();  }
  bool             empty() const { return m_SectionTable.empty(); }

  LDSection*       getSection(const std::string& pName);
  const LDSection* getSection(const std::string& pName) const;

  LDSymbol*       getSectionSymbol(const LDSection* pSection);
  const LDSymbol* getSectionSymbol(const LDSection* pSection) const;

  /// addSectionSymbol - create and add an section symbol for the output
  /// LDSection
  bool addSectionSymbol(LDSection& pOutputSection);

/// @}
/// @name Symbol Accessors
/// @{

  // -----  symbols  ----- //
  const SymbolTable& getSymbolTable() const { return m_SymbolTable; }
  SymbolTable&       getSymbolTable()       { return m_SymbolTable; }

  sym_iterator       sym_begin()       { return m_SymbolTable.begin();         }
  const_sym_iterator sym_begin() const { return m_SymbolTable.begin();         }
  sym_iterator       sym_end  ()       { return m_SymbolTable.end();           }
  const_sym_iterator sym_end  () const { return m_SymbolTable.end();           }
  size_t             sym_size () const { return m_SymbolTable.numOfSymbols();  }

  // -----  names  ----- //
  const NamePool& getNamePool() const { return m_NamePool; }
  NamePool&       getNamePool()       { return m_NamePool; }

/// @}
/// @name Relocation Accessors
/// @{

  // ----- relocation data ----- //
  RelocationDataTable& getRelocationDataTable()
  { return m_RelocDataTable; }

  const RelocationDataTable& getRelocationDataTable() const
  { return m_RelocDataTable; }

  reloc_data_iterator reloc_data_begin()
  { return m_RelocDataTable.begin(); }

  const_reloc_data_iterator reloc_data_begin() const
  { return m_RelocDataTable.begin(); }

  reloc_data_iterator reloc_data_end ()
  { return m_RelocDataTable.end(); }

  const_reloc_data_iterator reloc_data_end () const
  { return m_RelocDataTable.end(); }

private:
  /// sectCompare - hash compare function for LDSection*
  struct SectCompare
  {
    bool operator()(const LDSection* X, const LDSection* Y) const
    { return (X==Y); }
  };

  /// SectPtrHash - hash function for LDSection*
  struct SectPtrHash
  {
    size_t operator()(const LDSection* pKey) const
    {
      return (unsigned((uintptr_t)pKey) >> 4) ^
             (unsigned((uintptr_t)pKey) >> 9);
    }
  };

  typedef HashEntry<const LDSection*, LDSymbol*, SectCompare> SectHashEntryType;
  typedef HashTable<SectHashEntryType,
                    SectPtrHash,
                    EntryFactory<SectHashEntryType> > SectHashTableType;

private:
  std::string m_Name;
  ObjectList m_ObjectList;
  LibraryList m_LibraryList;
  InputTree m_MainTree;
  SectionTable m_SectionTable;
  SymbolTable m_SymbolTable;
  NamePool m_NamePool;
  RelocationDataTable m_RelocDataTable;
  SectHashTableType* m_pSectionSymbolMap;
};

} // namespace of mcld

#endif

