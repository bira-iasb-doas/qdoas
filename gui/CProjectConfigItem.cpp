
#include "CProjectConfigItem.h"


CProjectConfigItem::CProjectConfigItem()
{
  m_root = new CProjectConfigFolder("root", true);

  initializeMediateProject(&m_projProp);
}

CProjectConfigItem::~CProjectConfigItem()
{
  delete m_root;
}

void CProjectConfigItem::setProjectName(const QString &name)
{
  m_projectName = name;
}

const QString& CProjectConfigItem::projectName(void) const
{
  return m_projectName;
}

mediate_project_t* CProjectConfigItem::properties(void)
{
  // WARNING : allows (by design) poking at the internals ...

  return &m_projProp;
}

const mediate_project_t* CProjectConfigItem::properties(void) const
{
  return &m_projProp;
}

CProjectConfigTreeNode* CProjectConfigItem::rootNode(void)
{
  return m_root;
}

const CProjectConfigTreeNode* CProjectConfigItem::rootNode(void) const
{
  return m_root;
}

