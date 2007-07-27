#include "CWSplitter.h"

CWSplitter::CWSplitter(Qt::Orientation orientation, QWidget *parent) :
  QSplitter(orientation, parent),
  m_currentMode(0)
{
}

CWSplitter::~CWSplitter()
{
}

void CWSplitter::slotSetWidthMode(int newMode)
{
  if (count() && newMode != m_currentMode) {
    
    QList<int> tmpSizes(sizes());
    int currentSize = tmpSizes.front(); // current first widget size

    // store the current state
    std::map<int,int>::iterator it = m_modeToSizeMap.find(m_currentMode);
    if (it != m_modeToSizeMap.end())
      it->second = currentSize;
    else
      m_modeToSizeMap.insert(std::map<int,int>::value_type(m_currentMode, currentSize));

    // does the new mode have state to restore ??
    it = m_modeToSizeMap.find(newMode);
    if (it != m_modeToSizeMap.end()) {

      tmpSizes.front() = it->second;  // required size
      tmpSizes.back() += currentSize - it->second; // attempt to conserve the sum

      setSizes(tmpSizes);
    }
      
    // change the mode
    m_currentMode = newMode;
  }
}
