/* -*- C++ -*- */

#include "Element_City_Sidewalk.h"

namespace MFM
{
  template <class CC>
  u32 Element_City_Building<CC>::GetSidewalkType() const
  {
    return Element_City_Sidewalk<CC>::THE_INSTANCE.GetType();
  }

  template <class CC>
  bool Element_City_Building<CC>::DoNBSidewalkCase(EventWindow<CC>& window) const
  {
    SPoint sidewalk;
    WindowScanner<CC> scanner(window);
    u32 sidewalkCount = scanner.FindRandomInVonNeumann(GetSidewalkType(), sidewalk);

    if(sidewalkCount == 1)
    {
      Dir swDir = Dirs::FromOffset(sidewalk);

      SPoint adjs[2];

      Dirs::FillDir(adjs[0], Dirs::CWDir(Dirs::CWDir(swDir)));
      Dirs::FillDir(adjs[1], Dirs::CCWDir(Dirs::CCWDir(swDir)));

      for(u32 i = 0; i < 2; i++)
      {
        const T& at = window.GetRelativeAtom(adjs[i]);
        if(at.GetType() != TYPE())
        {
          T copyMe = window.GetCenterAtom();
          u32 idx = LargestVisibleIndex(window) + 1;

          if(idx <= GetMaxArea(copyMe))
          {
            SetAreaIndex(copyMe, idx);
            window.SetRelativeAtom(adjs[i], copyMe);
            return true;
          }
        }
      }
    }
    return false;
  }

  template <class CC>
  bool Element_City_Building<CC>::DoNBPerpGrowthCase(EventWindow<CC>& window) const
  {
    SPoint swPoint;
    WindowScanner<CC> scanner(window);
    u32 sidewalkCount = scanner.FindRandomInVonNeumann(GetSidewalkType(), swPoint);

    if(window.GetRandom().OneIn(10))
    {
      if(sidewalkCount == 1)
      {
        SPoint oppositePt;
        Dir swDir = Dirs::FromOffset(swPoint);
        Dirs::FillDir(oppositePt, Dirs::OppositeDir(swDir));

        if(window.GetRelativeAtom(oppositePt).GetType() ==
           Element_Empty<CC>::THE_INSTANCE.GetType())
        {
          T copyMe = window.GetCenterAtom();
          u32 idx = LargestVisibleIndex(window) + 1;

          if(idx <= GetMaxArea(copyMe))
          {
            SetAreaIndex(copyMe, idx);
            window.SetRelativeAtom(oppositePt, copyMe);
            return true;
          }
        }
      }
    }
    return false;
  }

  template <class CC>
  bool Element_City_Building<CC>::DoNBCornerGrowthCase(EventWindow<CC>& window) const
  {
    return false;
  }


  template <class CC>
  void Element_City_Building<CC>::SpawnNextBuilding(EventWindow<CC>& window) const
  {
    if(!DoNBSidewalkCase(window))
    {
      if(!DoNBPerpGrowthCase(window))
      {
        DoNBCornerGrowthCase(window);
      }
    }
  }
}
