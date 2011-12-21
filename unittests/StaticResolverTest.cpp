//===- implTest.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <mcld/LD/StaticResolver.h>
#include <mcld/LD/ResolveInfo.h>
#include <mcld/LD/ResolveInfoFactory.h>
#include "StaticResolverTest.h"

#include <iostream>

using namespace std;
using namespace mcld;
using namespace mcldtest;

// Constructor can do set-up work for all test here.
StaticResolverTest::StaticResolverTest()
{
  // create testee. modify it if need
  m_pResolver = new StaticResolver();
  m_pFactory = new ResolveInfoFactory();
}

// Destructor can do clean-up work that doesn't throw exceptions here.
StaticResolverTest::~StaticResolverTest()
{
  delete m_pResolver;
  delete m_pFactory;
}

// SetUp() will be called immediately before each test.
void StaticResolverTest::SetUp()
{
}

// TearDown() will be called immediately after each test.
void StaticResolverTest::TearDown()
{
}

//==========================================================================//
// Testcases
//
TEST_F( StaticResolverTest, MDEF ) {
  ResolveInfo* old_sym = m_pFactory->produce("abc");
  ResolveInfo* new_sym = m_pFactory->produce("abc");
  new_sym->setDesc(ResolveInfo::Define);
  old_sym->setDesc(ResolveInfo::Define);
  ASSERT_EQ( mcld::ResolveInfo::Define, new_sym->desc());
  ASSERT_EQ( mcld::ResolveInfo::Define, old_sym->desc());
  ASSERT_TRUE( mcld::ResolveInfo::define_flag == new_sym->info());
  ASSERT_TRUE( mcld::ResolveInfo::define_flag == old_sym->info());
  bool override = true;
  unsigned int result = m_pResolver->resolve(*old_sym, *new_sym, override);
  ASSERT_EQ( Resolver::Abort, result);
  ASSERT_FALSE( override );
  ASSERT_STREQ( "multiple definitions of `abc'.", m_pResolver->mesg().c_str() );
}

TEST_F( StaticResolverTest, DynDefAfterDynUndef ) {
  ResolveInfo* old_sym = m_pFactory->produce("abc");
  ResolveInfo* new_sym = m_pFactory->produce("abc");
  
  new_sym->setBinding(ResolveInfo::Global);
  old_sym->setBinding(ResolveInfo::Global);
  new_sym->setDesc(ResolveInfo::Undefined);
  old_sym->setDesc(ResolveInfo::Define);
  new_sym->setSource(true);
  old_sym->setSource(true);

  new_sym->setSize(0);
  new_sym->setValue(0);

  old_sym->setSize(1);
  old_sym->setValue(1);

  ASSERT_EQ( mcld::ResolveInfo::Global,    new_sym->binding());
  ASSERT_EQ( mcld::ResolveInfo::Global,    old_sym->binding());
  ASSERT_EQ( mcld::ResolveInfo::Undefined, new_sym->desc());
  ASSERT_EQ( mcld::ResolveInfo::Define,    old_sym->desc());

  bool override = false;
  unsigned int result = m_pResolver->resolve(*old_sym, *new_sym, override);
  ASSERT_EQ( Resolver::Success, result);
  ASSERT_FALSE( override );
  ASSERT_EQ(1, old_sym->size());
  ASSERT_EQ(1, old_sym->value());
}

TEST_F( StaticResolverTest, DynDefAfterDynDef ) {
  ResolveInfo* old_sym = m_pFactory->produce("abc");
  ResolveInfo* new_sym = m_pFactory->produce("abc");
  
  new_sym->setBinding(ResolveInfo::Global);
  old_sym->setBinding(ResolveInfo::Global);
  new_sym->setDesc(ResolveInfo::Define);
  old_sym->setDesc(ResolveInfo::Define);
  new_sym->setSource(true);
  old_sym->setSource(true);

  new_sym->setSize(0);
  new_sym->setValue(0);

  old_sym->setSize(1);
  old_sym->setValue(1);

  ASSERT_EQ( mcld::ResolveInfo::Global, new_sym->binding());
  ASSERT_EQ( mcld::ResolveInfo::Global, old_sym->binding());
  ASSERT_EQ( mcld::ResolveInfo::Define, new_sym->desc());
  ASSERT_EQ( mcld::ResolveInfo::Define, old_sym->desc());

  bool override = false;
  unsigned int result = m_pResolver->resolve(*old_sym, *new_sym, override);
  ASSERT_EQ( Resolver::Success, result);
  ASSERT_FALSE( override );
  ASSERT_EQ(1, old_sym->size());
  ASSERT_EQ(1, old_sym->value());
}

TEST_F( StaticResolverTest, DynUndefAfterDynUndef ) {
  ResolveInfo* old_sym = m_pFactory->produce("abc");
  ResolveInfo* new_sym = m_pFactory->produce("abc");
  
  new_sym->setBinding(ResolveInfo::Global);
  old_sym->setBinding(ResolveInfo::Global);
  new_sym->setDesc(ResolveInfo::Undefined);
  old_sym->setDesc(ResolveInfo::Undefined);
  new_sym->setSource(true);
  old_sym->setSource(true);

  new_sym->setSize(0);
  new_sym->setValue(0);

  old_sym->setSize(1);
  old_sym->setValue(1);

  ASSERT_EQ( mcld::ResolveInfo::Global,    new_sym->binding());
  ASSERT_EQ( mcld::ResolveInfo::Global,    old_sym->binding());
  ASSERT_EQ( mcld::ResolveInfo::Undefined, new_sym->desc());
  ASSERT_EQ( mcld::ResolveInfo::Undefined, old_sym->desc());

  bool override = false;
  unsigned int result = m_pResolver->resolve(*old_sym, *new_sym, override);
  ASSERT_EQ( Resolver::Success, result);
  ASSERT_FALSE( override );
  ASSERT_EQ(1, old_sym->size());
  ASSERT_EQ(1, old_sym->value());
}

TEST_F( StaticResolverTest, OverrideWeakByGlobal )
{
  ResolveInfo* old_sym = m_pFactory->produce("abc");
  ResolveInfo* new_sym = m_pFactory->produce("abc");
  
  new_sym->setBinding(ResolveInfo::Global);
  old_sym->setBinding(ResolveInfo::Weak);
  new_sym->setSize(0);
  old_sym->setSize(1);
  new_sym->setValue(0);
  old_sym->setValue(1);

  ASSERT_EQ( mcld::ResolveInfo::Global, new_sym->binding());
  ASSERT_EQ( mcld::ResolveInfo::Weak, old_sym->binding());

  ASSERT_TRUE( mcld::ResolveInfo::global_flag == new_sym->info());
  ASSERT_TRUE( mcld::ResolveInfo::weak_flag == old_sym->info());
  bool override = false;
  unsigned int result = m_pResolver->resolve(*old_sym, *new_sym, override);
  ASSERT_EQ( Resolver::Success, result);
  ASSERT_TRUE( override );
  ASSERT_EQ(0, old_sym->size());
  ASSERT_EQ(0, old_sym->value());
}

TEST_F( StaticResolverTest, DynWeakAfterDynDef ) {
  ResolveInfo* old_sym = m_pFactory->produce("abc");
  ResolveInfo* new_sym = m_pFactory->produce("abc");
  
  old_sym->setBinding(ResolveInfo::Weak);
  new_sym->setBinding(ResolveInfo::Global);

  new_sym->setSource(true);
  old_sym->setSource(true);

  old_sym->setDesc(ResolveInfo::Define);
  new_sym->setDesc(ResolveInfo::Define);

  new_sym->setSize(0);
  new_sym->setValue(0);

  old_sym->setSize(1);
  old_sym->setValue(1);

  ASSERT_EQ( mcld::ResolveInfo::Weak,   old_sym->binding());
  ASSERT_EQ( mcld::ResolveInfo::Global, new_sym->binding());
  ASSERT_EQ( mcld::ResolveInfo::Define, old_sym->desc());
  ASSERT_EQ( mcld::ResolveInfo::Define, new_sym->desc());

  bool override = false;
  unsigned int result = m_pResolver->resolve(*old_sym, *new_sym, override);
  ASSERT_EQ( Resolver::Success, result);
  ASSERT_FALSE( override );
  ASSERT_EQ(1, old_sym->size());
  ASSERT_EQ(1, old_sym->value());
}

TEST_F( StaticResolverTest, MarkByBiggerCommon )
{
  ResolveInfo* old_sym = m_pFactory->produce("abc");
  ResolveInfo* new_sym = m_pFactory->produce("abc");
  
  new_sym->setDesc(ResolveInfo::Common);
  old_sym->setDesc(ResolveInfo::Common);
  new_sym->setSize(999);
  old_sym->setSize(0);
  new_sym->setValue(999);
  old_sym->setValue(111);

  ASSERT_EQ( mcld::ResolveInfo::Common, new_sym->desc());
  ASSERT_EQ( mcld::ResolveInfo::Common, old_sym->desc());

  ASSERT_TRUE( mcld::ResolveInfo::common_flag == new_sym->info());
  ASSERT_TRUE( mcld::ResolveInfo::common_flag == old_sym->info());
  bool override = true;
  unsigned int result = m_pResolver->resolve(*old_sym, *new_sym, override);
  ASSERT_EQ( Resolver::Success, result);
  ASSERT_FALSE( override );
  ASSERT_EQ(999, old_sym->size());
  ASSERT_EQ(111, old_sym->value());
}

TEST_F( StaticResolverTest, OverrideByBiggerCommon )
{
  ResolveInfo* old_sym = m_pFactory->produce("abc");
  ResolveInfo* new_sym = m_pFactory->produce("abc");
  
  new_sym->setDesc(ResolveInfo::Common);
  old_sym->setDesc(ResolveInfo::Common);
  old_sym->setBinding(ResolveInfo::Weak);
  new_sym->setSize(999);
  old_sym->setSize(0);
  new_sym->setValue(999);
  old_sym->setValue(111);

  ASSERT_EQ( ResolveInfo::Common, new_sym->desc());
  ASSERT_EQ( ResolveInfo::Common, old_sym->desc());
  ASSERT_EQ( ResolveInfo::Weak, old_sym->binding());

  ASSERT_TRUE( ResolveInfo::common_flag == new_sym->info());
  ASSERT_TRUE( (ResolveInfo::weak_flag | ResolveInfo::common_flag) == old_sym->info());

  bool override = false;
  unsigned int result = m_pResolver->resolve(*old_sym, *new_sym, override);
  ASSERT_EQ( Resolver::Success, result);
  ASSERT_TRUE( override );
  ASSERT_EQ(999, old_sym->size());
  ASSERT_EQ(999, old_sym->value());
}

TEST_F( StaticResolverTest, OverrideCommonByDefine)
{
  ResolveInfo* old_sym = m_pFactory->produce("abc");
  ResolveInfo* new_sym = m_pFactory->produce("abc");
  
  old_sym->setDesc(ResolveInfo::Common);
  old_sym->setSize(0);
  old_sym->setValue(111);

  new_sym->setDesc(ResolveInfo::Define);
  new_sym->setSize(999);
  new_sym->setValue(999);

  ASSERT_EQ( ResolveInfo::Define, new_sym->desc());
  ASSERT_EQ( ResolveInfo::Common, old_sym->desc());

  ASSERT_TRUE( ResolveInfo::define_flag == new_sym->info());
  ASSERT_TRUE( ResolveInfo::common_flag == old_sym->info());

  bool override = false;
  unsigned int result = m_pResolver->resolve(*old_sym, *new_sym, override);
  ASSERT_EQ(Resolver::Warning, result);
  ASSERT_TRUE( override );
  ASSERT_EQ(999, old_sym->size());
  ASSERT_EQ(999, old_sym->value());
  
  ASSERT_STREQ("definition of 'abc' is overriding common.", m_pResolver->mesg().c_str() );

}

TEST_F( StaticResolverTest, SetUpDesc)
{
  ResolveInfo* sym = m_pFactory->produce("abc");
  
  sym->setIsSymbol(true);

//  ASSERT_FALSE( sym->isSymbol() );
  ASSERT_TRUE( sym->isSymbol() );
  ASSERT_TRUE( sym->isGlobal() );
  ASSERT_FALSE( sym->isWeak() );
  ASSERT_FALSE( sym->isLocal() );
  ASSERT_FALSE( sym->isDefine() );
  ASSERT_TRUE( sym->isUndef() );
  ASSERT_FALSE( sym->isDyn() );
  ASSERT_FALSE( sym->isCommon() );
  ASSERT_FALSE( sym->isIndirect() );
  ASSERT_EQ( ResolveInfo::NoType, sym->type());
  ASSERT_EQ( 0, sym->desc() );
  ASSERT_EQ( 0, sym->binding() );
  ASSERT_EQ( 0, sym->other() );

  sym->setIsSymbol(false);
  ASSERT_FALSE( sym->isSymbol() );
//  ASSERT_TRUE( sym->isSymbol() );
  ASSERT_TRUE( sym->isGlobal() );
  ASSERT_FALSE( sym->isWeak() );
  ASSERT_FALSE( sym->isLocal() );
  ASSERT_FALSE( sym->isDefine() );
  ASSERT_TRUE( sym->isUndef() );
  ASSERT_FALSE( sym->isDyn() );
  ASSERT_FALSE( sym->isCommon() );
  ASSERT_FALSE( sym->isIndirect() );
  ASSERT_EQ( ResolveInfo::NoType, sym->type());
  ASSERT_EQ( 0, sym->desc() );
  ASSERT_EQ( 0, sym->binding() );
  ASSERT_EQ( 0, sym->other() );

  sym->setDesc(ResolveInfo::Define);
  ASSERT_FALSE( sym->isSymbol() );
//  ASSERT_TRUE( sym->isSymbol() );
  ASSERT_TRUE( sym->isGlobal() );
  ASSERT_FALSE( sym->isWeak() );
  ASSERT_FALSE( sym->isLocal() );
  ASSERT_TRUE( sym->isDefine() );
  ASSERT_FALSE( sym->isUndef() );
  ASSERT_FALSE( sym->isDyn() );
  ASSERT_FALSE( sym->isCommon() );
  ASSERT_FALSE( sym->isIndirect() );
  ASSERT_EQ( ResolveInfo::NoType, sym->type());
  ASSERT_EQ( ResolveInfo::Define, sym->desc() );
  ASSERT_EQ( 0, sym->binding() );
  ASSERT_EQ( 0, sym->other() );

  sym->setDesc(ResolveInfo::Common);
  ASSERT_FALSE( sym->isSymbol() );
//  ASSERT_TRUE( sym->isSymbol() );
  ASSERT_TRUE( sym->isGlobal() );
  ASSERT_FALSE( sym->isWeak() );
  ASSERT_FALSE( sym->isLocal() );
  ASSERT_FALSE( sym->isDyn() );
  ASSERT_FALSE( sym->isDefine() );
  ASSERT_FALSE( sym->isUndef() );
  ASSERT_TRUE( sym->isCommon() );
  ASSERT_FALSE( sym->isIndirect() );
  ASSERT_EQ( ResolveInfo::NoType, sym->type());
  ASSERT_EQ( ResolveInfo::Common, sym->desc() );
  ASSERT_EQ( 0, sym->binding() );
  ASSERT_EQ( 0, sym->other() );

  sym->setDesc(ResolveInfo::Indirect);
  ASSERT_FALSE( sym->isSymbol() );
  ASSERT_TRUE( sym->isGlobal() );
  ASSERT_FALSE( sym->isWeak() );
  ASSERT_FALSE( sym->isLocal() );
  ASSERT_FALSE( sym->isDyn() );
  ASSERT_FALSE( sym->isDefine() );
  ASSERT_FALSE( sym->isUndef() );
  ASSERT_FALSE( sym->isCommon() );
  ASSERT_TRUE( sym->isIndirect() );
  ASSERT_EQ( ResolveInfo::NoType, sym->type());
  ASSERT_EQ( ResolveInfo::Indirect, sym->desc() );
  ASSERT_EQ( 0, sym->binding() );
  ASSERT_EQ( 0, sym->other() );

  sym->setDesc(ResolveInfo::Undefined);
  ASSERT_FALSE( sym->isSymbol() );
  ASSERT_TRUE( sym->isGlobal() );
  ASSERT_FALSE( sym->isWeak() );
  ASSERT_FALSE( sym->isLocal() );
  ASSERT_FALSE( sym->isDyn() );
  ASSERT_TRUE( sym->isUndef() );
  ASSERT_FALSE( sym->isDefine() );
  ASSERT_FALSE( sym->isCommon() );
  ASSERT_FALSE( sym->isIndirect() );
  ASSERT_EQ( ResolveInfo::NoType, sym->type());
  ASSERT_EQ( 0, sym->desc() );
  ASSERT_EQ( 0, sym->binding() );
  ASSERT_EQ( 0, sym->other() );
}

TEST_F( StaticResolverTest, SetUpBinding)
{
  ResolveInfo* sym = m_pFactory->produce("abc");
  
  sym->setIsSymbol(true);

//  ASSERT_FALSE( sym->isSymbol() );
  ASSERT_TRUE( sym->isSymbol() );
  ASSERT_TRUE( sym->isGlobal() );
  ASSERT_FALSE( sym->isWeak() );
  ASSERT_FALSE( sym->isLocal() );
  ASSERT_FALSE( sym->isDefine() );
  ASSERT_TRUE( sym->isUndef() );
  ASSERT_FALSE( sym->isDyn() );
  ASSERT_FALSE( sym->isCommon() );
  ASSERT_FALSE( sym->isIndirect() );
  ASSERT_EQ( ResolveInfo::NoType, sym->type());
  ASSERT_EQ( 0, sym->desc() );
  ASSERT_EQ( 0, sym->binding() );
  ASSERT_EQ( 0, sym->other() );

  sym->setBinding(ResolveInfo::Global);
  ASSERT_TRUE( sym->isSymbol() );
  ASSERT_TRUE( sym->isGlobal() );
  ASSERT_FALSE( sym->isWeak() );
  ASSERT_FALSE( sym->isLocal() );
  ASSERT_FALSE( sym->isDefine() );
  ASSERT_TRUE( sym->isUndef() );
  ASSERT_FALSE( sym->isDyn() );
  ASSERT_FALSE( sym->isCommon() );
  ASSERT_FALSE( sym->isIndirect() );
  ASSERT_EQ( ResolveInfo::NoType, sym->type());
  ASSERT_EQ( 0, sym->desc() );
  ASSERT_EQ( ResolveInfo::Global, sym->binding() );
  ASSERT_EQ( 0, sym->other() );

  sym->setBinding(ResolveInfo::Weak);
  ASSERT_TRUE( sym->isSymbol() );
  ASSERT_FALSE( sym->isGlobal() );
  ASSERT_TRUE( sym->isWeak() );
  ASSERT_FALSE( sym->isLocal() );
  ASSERT_FALSE( sym->isDyn() );
  ASSERT_FALSE( sym->isDefine() );
  ASSERT_TRUE( sym->isUndef() );
  ASSERT_FALSE( sym->isCommon() );
  ASSERT_FALSE( sym->isIndirect() );
  ASSERT_EQ( ResolveInfo::NoType, sym->type());
  ASSERT_EQ( 0, sym->desc() );
  ASSERT_EQ( ResolveInfo::Weak, sym->binding() );
  ASSERT_EQ( 0, sym->other() );

  sym->setBinding(ResolveInfo::Local);
  ASSERT_TRUE( sym->isSymbol() );
  ASSERT_FALSE( sym->isGlobal() );
  ASSERT_FALSE( sym->isWeak() );
  ASSERT_TRUE( sym->isLocal() );
  ASSERT_FALSE( sym->isDyn() );
  ASSERT_FALSE( sym->isDefine() );
  ASSERT_TRUE( sym->isUndef() );
  ASSERT_FALSE( sym->isCommon() );
  ASSERT_FALSE( sym->isIndirect() );
  ASSERT_EQ( ResolveInfo::NoType, sym->type());
  ASSERT_EQ( 0, sym->desc() );
  ASSERT_EQ( ResolveInfo::Local, sym->binding() );
  ASSERT_EQ( 0, sym->other() );
}

