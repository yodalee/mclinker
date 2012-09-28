//===- THMToARMStub.cpp ---------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "THMToARMStub.h"
#include "ARMLDBackend.h"

#include <llvm/Support/ELF.h>
#include <mcld/LD/ResolveInfo.h>
#include <mcld/LD/LDSymbol.h>
#include <mcld/Fragment/Relocation.h>

using namespace mcld;

//===----------------------------------------------------------------------===//
// THMToARMStub
//===----------------------------------------------------------------------===//
const uint32_t THMToARMStub::PIC_TEMPLATE[] = {
  0x46c04778, // bx    pc ... nop
  0xe59fc000, // ldr   ip, [pc, #0]
  0xe08cf00f, // add   pc, ip, pc
  0x0         // dcd   R_ARM_REL32(X)
};

const uint32_t THMToARMStub::TEMPLATE[] = {
  0x46c04778, // bx    pc ... nop
  0xe51ff004, // ldr   pc, [pc, #-4]
  0x0         // dcd   R_ARM_ABS32(X)
};

THMToARMStub::THMToARMStub()
 : Stub(), m_Name("T2A_prototype"), m_pData(NULL), m_Size(0x0)
{
}

THMToARMStub::THMToARMStub(bool pIsOutputPIC)
 : Stub(), m_Name("T2A_veneer"), m_pData(NULL), m_Size(0x0)
{
  if (pIsOutputPIC) {
    m_Size = sizeof(PIC_TEMPLATE);
    m_pData = new uint32_t[m_Size];
    std::memcpy(m_pData, PIC_TEMPLATE, m_Size);
    addFixup(12u, -4, llvm::ELF::R_ARM_REL32);
  }
  else {
    m_Size = sizeof(TEMPLATE);
    m_pData = new uint32_t[m_Size];
    std::memcpy(m_pData, TEMPLATE, m_Size);
    addFixup(8u, 0x0, llvm::ELF::R_ARM_ABS32);
  }
}

THMToARMStub::~THMToARMStub()
{
  if (NULL != m_pData)
    delete[] m_pData;
}

bool THMToARMStub::isMyDuty(const class Relocation& pReloc,
                            uint64_t pSource,
                            uint64_t pTargetSymValue) const
{
  bool result = false;
  switch (pReloc.type()) {
    case llvm::ELF::R_ARM_THM_CALL: {
      if ((pTargetSymValue & 0x1) != 0x0)
        break;

      if ((pReloc.symInfo()->outSymbol()->fragRef() != NULL)) { 
        const Fragment* frag = pReloc.symInfo()->outSymbol()->fragRef()->frag();
        if (classof(frag)) {
          result = false;
          break;
        }
      }

      uint64_t dest = pTargetSymValue + pReloc.addend() + 4u;
      int64_t branch_offset = static_cast<int64_t>(dest) - pSource;
      if ((branch_offset > ARMGNULDBackend::THM_MAX_FWD_BRANCH_OFFSET) ||
          (branch_offset < ARMGNULDBackend::THM_MAX_BWD_BRANCH_OFFSET))
        result = true;
      break;
    }
    case llvm::ELF::R_ARM_THM_JUMP24: {
      if ((pTargetSymValue & 0x1) != 0x0)
        break;

      if ((pReloc.symInfo()->outSymbol()->fragRef() != NULL)) { 
        const Fragment* frag = pReloc.symInfo()->outSymbol()->fragRef()->frag();
        if (classof(frag)) {
          result = false;
          break;
        }
      }

      result = true;
      break;
    }
    default:
      break;
  }
  return result;
}

const std::string& THMToARMStub::name() const
{
  return m_Name;
}

uint8_t* THMToARMStub::getContent()
{
  return reinterpret_cast<uint8_t*>(m_pData);
}

const uint8_t* THMToARMStub::getContent() const
{
  return reinterpret_cast<const uint8_t*>(m_pData);
}

size_t THMToARMStub::size() const
{
  return m_Size;
}

size_t THMToARMStub::alignment() const
{
  return 4u;
}

// for T bit of this stub
uint64_t THMToARMStub::initSymValue() const
{
  return 0x1;
}

Stub* THMToARMStub::doClone(bool pIsOutputPIC)
{
  return new THMToARMStub(pIsOutputPIC);
}
