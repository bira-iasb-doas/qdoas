/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef _CPROJECTCONFIGITEM_H_GUARD
#define _CPROJECTCONFIGITEM_H_GUARD

#include <QString>
#include <QList>

#include "mediate.h"
#include "CProjectConfigTreeNode.h"

class CProjectConfigItem
{
 public:
  CProjectConfigItem();
  ~CProjectConfigItem();

  void setProjectName(const QString &name);
  const QString& projectName(void) const;

  mediate_project_t* properties(void); // WARNING : allows (by design) poking at the internals ...
  const mediate_project_t* properties(void) const;

  CProjectConfigTreeNode* rootNode(void); // WARNING : allows (by design) poking at the internals ...
  const CProjectConfigTreeNode* rootNode(void) const;

 private:
  QString m_projectName;
  mediate_project_t m_projProp;
  CProjectConfigTreeNode *m_root;
};

class CSiteConfigItem
{
 public:
  CSiteConfigItem();
  ~CSiteConfigItem();

  void setSiteName(const QString &name);
  void setAbbreviation(const QString &abbreviation);
  void setLongitude(double longitude);
  void setLatitude(double latitude);
  void setAltitude(double altitude);
  
  const QString& siteName(void) const;
  const QString& abbreviation(void) const;
  double longitude(void) const;
  double latitude(void) const;
  double altitude(void) const;

 private:
  QString m_siteName, m_abbreviation;
  double m_longitude, m_latitude, m_altitude;
};

inline void CSiteConfigItem::setSiteName(const QString &siteName) { m_siteName = siteName; }
inline void CSiteConfigItem::setAbbreviation(const QString &abbreviation) { m_abbreviation = abbreviation; }
inline void CSiteConfigItem::setLongitude(double longitude) { m_longitude = longitude; }
inline void CSiteConfigItem::setLatitude(double latitude) { m_latitude = latitude; }
inline void CSiteConfigItem::setAltitude(double altitude) { m_altitude = altitude; }

inline const QString& CSiteConfigItem::siteName(void) const { return m_siteName; }
inline const QString& CSiteConfigItem::abbreviation(void) const { return m_abbreviation; }
inline double CSiteConfigItem::longitude(void) const { return m_longitude; }
inline double CSiteConfigItem::latitude(void) const { return m_latitude; }
inline double CSiteConfigItem::altitude(void) const { return m_altitude; }

#endif
