/*****************************************************************************
 *   The MCLinker Project, Copyright (C), 2011 -                             *
 *   Embedded and Web Computing Lab, National Taiwan University              *
 *   MediaTek, Inc.                                                          *
 *                                                                           *
 *   Jush Lu <Jush.Lu@mediatek.com>                                          *
 *   Luba Tang <lubatang@mediatek.com>                                       *
 ****************************************************************************/
#include <mcld/MC/ContextFactory.h>

using namespace mcld;

//===---------------------------------------------------------------------===//
// MCLDContextFactory
ContextFactory::ContextFactory(size_t pNum)
  : UniqueGCFactoryBase<sys::fs::Path, MCLDContext, 0>(pNum) {
}

ContextFactory::~ContextFactory()
{
}

