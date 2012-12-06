//===- OutputRelocSection.cpp ---------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <mcld/Target/OutputRelocSection.h>

#include <mcld/LD/LDSection.h>
#include <mcld/LD/RelocationFactory.h>
#include <mcld/Module.h>
#include <mcld/Support/MsgHandling.h>
#include <mcld/IRBuilder.h>

#include <llvm/Support/Casting.h>

using namespace mcld;

//===----------------------------------------------------------------------===//
// OutputRelocSection
//===----------------------------------------------------------------------===//
OutputRelocSection::OutputRelocSection(Module& pModule,
                                       LDSection& pSection,
                                       unsigned int pEntrySize)
  : m_Module(pModule),
    m_pSection(&pSection),
    m_pRelocData(NULL),
    m_EntryBytes(pEntrySize),
    m_isVisit(false),
    m_ValidEntryIterator(){
  assert(!pSection.hasRelocData() && "Given section is not a relocation section");
  m_pRelocData = IRBuilder::CreateRelocData(pSection);
}

OutputRelocSection::~OutputRelocSection()
{
}

void OutputRelocSection::reserveEntry(size_t pNum)
{
  for(size_t i=0; i<pNum; i++) {
    m_pRelocData->append(*Relocation::Create());
    // update section size
    m_pSection->setSize(m_pSection->size() + m_EntryBytes);
  }
}

Relocation* OutputRelocSection::consumeEntry()
{
  // first time visit this function, set m_ValidEntryIterator to
  // Fragments.begin()
  if(!m_isVisit) {
    assert(!m_pRelocData->getRelocationList().empty() &&
             "DynRelSection contains no entries.");
    m_ValidEntryIterator = m_pRelocData->begin();
    m_isVisit = true;
  }
  else {
    // Add m_ValidEntryIterator here instead of at the end of this function.
    // We may reserve an entry and then consume it immediately, e.g. for COPY
    // relocation, so we need to avoid setting this iterator to
    // RelocData->end() in any case, or when reserve and consume again,
    // ++m_ValidEntryIterator will still be RelocData->end().
    ++m_ValidEntryIterator;
  }
  assert(m_ValidEntryIterator != m_pRelocData->end() &&
         "No empty relocation entry for the incoming symbol.");

  return &(*m_ValidEntryIterator);
}

void OutputRelocSection::finalizeSectionSize()
{
  m_pSection->setSize(m_pRelocData->size() * m_EntryBytes);
}

bool OutputRelocSection::addSymbolToDynSym(LDSymbol& pSymbol)
{
  m_Module.getSymbolTable().changeLocalToTLS(pSymbol);
  return true;
}
