//===- MCLinker.cpp -----------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the MCLinker class
//
//===----------------------------------------------------------------------===//

#include <mcld/MC/MCLinker.h>
#include <mcld/MC/MCLDInput.h>
#include <mcld/MC/MCLDInfo.h>
#include <mcld/LD/Resolver.h>
#include <mcld/LD/LDContext.h>
#include <mcld/LD/LDSymbol.h>
#include <mcld/LD/LDSectionFactory.h>
#include <mcld/LD/SectionMap.h>
#include <mcld/Target/TargetLDBackend.h>
#include <llvm/Support/raw_ostream.h>

using namespace mcld;

MCLinker::MCLinker(TargetLDBackend& pBackend,
                   MCLDInfo& pInfo,
                   LDContext& pContext,
                   SectionMap& pSectionMap,
                   const Resolver& pResolver)
: m_Backend(pBackend),
  m_Info(pInfo),
  m_Output(pContext),
  m_SectionMap(pSectionMap),
  m_LDSymbolFactory(128),
  m_LDSectHdrFactory(10), // the average number of sections. (assuming 10.)
  m_LDSectDataFactory(10),
  m_SectionMerger(pSectionMap, pContext),
  m_StrSymPool(pResolver, 128)
{
  m_Info.setNamePool(m_StrSymPool);
}

MCLinker::~MCLinker()
{
}

/// addSymbol - add a symbol and resolve it immediately
LDSymbol* MCLinker::addGlobalSymbol(const llvm::StringRef& pName,
                                    bool pIsDyn,
                                    ResolveInfo::Type pType,
                                    ResolveInfo::Desc pDesc,
                                    ResolveInfo::Binding pBinding,
                                    ResolveInfo::SizeType pSize,
                                    const MCFragmentRef& pFragmentRef,
                                    ResolveInfo::Visibility pVisibility)
{
  if (pBinding == ResolveInfo::Local)
    return NULL;

  // <resolved_info, exist?, override>
  Resolver::Result resolved_result;
  m_StrSymPool.insertSymbol(pName, pIsDyn, pType, pDesc, pBinding, pSize, pVisibility,
                            resolved_result);

  // the return ResolveInfo should not NULL
  assert(NULL != resolved_result.info);

  // create a LDSymbol for the input file.
  LDSymbol* input_sym = m_LDSymbolFactory.allocate();
  new (input_sym) LDSymbol();
  input_sym->setResolveInfo(*resolved_result.info);
  input_sym->setFragmentRef(pFragmentRef);

  // if it is a new symbol, create a LDSymbol for the output
  if (!resolved_result.existent) {
    LDSymbol* output_sym = m_LDSymbolFactory.allocate();
    new (output_sym) LDSymbol();
    output_sym->setResolveInfo(*resolved_result.info);
    output_sym->setFragmentRef(pFragmentRef);
    m_Output.symtab().push_back(output_sym);
  }
  return input_sym;
}

LDSymbol* MCLinker::addLocalSymbol(const llvm::StringRef& pName,
                                   ResolveInfo::Type pType,
                                   ResolveInfo::Desc pDesc,
                                   ResolveInfo::SizeType pSize,
                                   const MCFragmentRef& pFragmentRef,
                                   ResolveInfo::Visibility pVisibility)
{
  ResolveInfo* resolved_info =  m_StrSymPool.createSymbol(pName,
                                                          false,
                                                          pType,
                                                          pDesc,
                                                          ResolveInfo::Local,
                                                          pSize,
                                                          pVisibility);

  // the return ResolveInfo should not NULL
  assert(NULL != resolved_info);

  // create a LDSymbol for the input file.
  LDSymbol* input_sym = m_LDSymbolFactory.allocate();
  new (input_sym) LDSymbol();
  input_sym->setResolveInfo(*resolved_info);
  input_sym->setFragmentRef(pFragmentRef);

  // create a LDSymbol for the output
  LDSymbol* output_sym = m_LDSymbolFactory.allocate();
  new (output_sym) LDSymbol();
  output_sym->setResolveInfo(*resolved_info);
  output_sym->setFragmentRef(pFragmentRef);
  m_Output.symtab().push_back(output_sym);

  return input_sym;
}

LDSection& MCLinker::createSectHdr(const std::string& pName,
                                   LDFileFormat::Kind pKind,
                                   uint32_t pType,
                                   uint32_t pFlag)
{
  // for user such as reader, standard/target fromat
  LDSection* result =
    m_LDSectHdrFactory.produce(pName, pKind, pType, pFlag);

  // check if we need to create a output section for output LDContext
  std::string sect_name = m_SectionMap.getOutputSectName(pName);
  LDSection* output_sect = m_Output.getSection(sect_name);

  if (NULL == output_sect) {
  // create a output section and push it into output LDContext
    output_sect =
      m_LDSectHdrFactory.produce(sect_name, pKind, pType, pFlag);
    m_Output.getSectionTable().push_back(output_sect);
    m_SectionMerger.addMapping(pName, output_sect);
  }
  return *result;
}

/// getOrCreateOutputSectHdr - for reader and standard/target format to get
/// or create the output's section header
LDSection& MCLinker::getOrCreateOutputSectHdr(const std::string& pName,
                                              LDFileFormat::Kind pKind,
                                              uint32_t pType,
                                              uint32_t pFlag)
{
  // check if we need to create a output section for output LDContext
  std::string sect_name = m_SectionMap.getOutputSectName(pName);
  LDSection* output_sect = m_Output.getSection(sect_name);

  if (NULL == output_sect) {
  // create a output section and push it into output LDContext
    output_sect =
      m_LDSectHdrFactory.produce(sect_name, pKind, pType, pFlag);
    m_Output.getSectionTable().push_back(output_sect);
    m_SectionMerger.addMapping(pName, output_sect);
  }
  return *output_sect;
}

llvm::MCSectionData& MCLinker::getOrCreateSectData(LDSection& pSection)
{
  // if there is already a section data pointed by section, return it.
  llvm::MCSectionData* sect_data = pSection.getSectionData();
  if (NULL != sect_data) {
    m_Layout.addInputRange(*sect_data, pSection);
    return *sect_data;
  }

  // create a new section data
  // try to get one from output LDSection
  LDSection* output_sect =
    m_SectionMerger.getOutputSectHdr(pSection.name());

  assert(NULL != output_sect);

  sect_data = output_sect->getSectionData();

  if (NULL != sect_data) {
    pSection.setSectionData(sect_data);
    m_Layout.addInputRange(*sect_data, pSection);
    return *sect_data;
  }

  // if the output LDSection also has no MCSectionData, then create one.
  sect_data = m_LDSectDataFactory.allocate();
  new (sect_data) llvm::MCSectionData(*output_sect);
  pSection.setSectionData(sect_data);
  output_sect->setSectionData(sect_data);

  m_Layout.addInputRange(*sect_data, pSection);
  return *sect_data;
}

/// addRelocation - add a relocation entry in MCLinker (only for object file)
Relocation* MCLinker::addRelocation(Relocation::Type pType,
                                    LDSymbol& pSymbol,
                                    Relocation::Address pOffset,
                                    Relocation::Address pAddend)
{
  // TODO
  return NULL;
}

bool MCLinker::layout()
{
  return m_Layout.layout(m_Output, m_Backend);
}

