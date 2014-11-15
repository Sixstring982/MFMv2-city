/* -*- C++ -*- */

#include "Element_City_Building.h"

namespace MFM
{
  template <class CC>
  void Element_City_Sidewalk<CC>::DoBuildingBehavior(EventWindow<CC>& window) const
  {
    WindowScanner<CC> scanner(window);
    SPoint emptyPt;
    if(scanner.FindEmptyInVonNeumann(emptyPt) > 0)
    {
      window.SetRelativeAtom(emptyPt,
                             Element_City_Building<CC>::THE_INSTANCE.GetDefaultAtom());
    }
  }

  template <class CC>
  u32 Element_City_Sidewalk<CC>::GetBuildingType() const
  {
    return Element_City_Building<CC>::THE_INSTANCE.GetType();
  }

  template <class CC>
  u32 Element_City_Sidewalk<CC>::GetStreetType() const
  {
    return Element_City_Street<CC>::THE_INSTANCE.GetType();
  }

  template <class CC>
  void Element_City_Sidewalk<CC>::UpdateBuildingCounts(EventWindow<CC>& window) const
  {
    /* Find a random sidewalk I'm directly next to */
    WindowScanner<CC> scanner(window);
    SPoint borderSidewalk;
    u32 sidewalkCount;
    SPoint borderBuilding;
    u32 buildingCount;
    SPoint borderStreet;
    u32 streetCount;

    scanner.FindRandomAtoms(1, 3,
                            &borderSidewalk, TYPE(), &sidewalkCount,
                            &borderBuilding, GetBuildingType(), &buildingCount,
                            &borderStreet, GetStreetType(), &streetCount);

    if(buildingCount > 0)
    {
      /* Set my distance from this building to 0 */
      T newMe = window.GetCenterAtom();
      SetBuildingCount(newMe,
                       Element_City_Building<CC>::THE_INSTANCE.
                           GetSubType(window.GetRelativeAtom(borderBuilding)),
                       0);

      window.SetCenterAtom(newMe);
    }

    if(sidewalkCount > 0)
    {
      if(IsReadyToBuild(window.GetRelativeAtom(borderSidewalk)))
      {
        u32 count = 0;
        for(u32 i = 0; i < CityConstants::CITY_BUILDING_COUNT; i++)
        {
          count = MIN(GetBuildingCount(window.GetCenterAtom(), i),
                      GetBuildingCount(window.GetRelativeAtom(borderSidewalk), i));

          T newMe = window.GetCenterAtom();
          SetBuildingCount(newMe, i, count);
          window.SetCenterAtom(newMe);
        }
      }
    }

    if(streetCount > 0)
    {
      /* See if there's a sidewalk on the other side of the street */
      borderStreet *= 2;

      if(window.IsLiveSite(borderStreet))
      {
        if(window.GetRelativeAtom(borderStreet).GetType() == TYPE())
        {
          if(IsReadyToBuild(window.GetRelativeAtom(borderStreet)))
          {
            /* Sure is! Do the min trick, but add 1 to its counts. */
            u32 count;
            for(u32 i = 0; i < CityConstants::CITY_BUILDING_COUNT; i++)
            {
              count = MIN(GetBuildingCount(window.GetCenterAtom(), i),
                          GetBuildingCount(window.GetRelativeAtom(borderStreet), i) + 1);

              if(count < 4) /* Can't store higher res than 2 bits */
              {
                T newMe = window.GetCenterAtom();
                SetBuildingCount(newMe, i, count);
                window.SetCenterAtom(newMe);
              }
            }
          }
	  if (IsRebuilding(window.GetRelativeAtom(borderStreet)) &&
	      (GetBuildingTimer(window.GetRelativeAtom(borderStreet)) <
	       MAX_TIMER_VALUE * 0.1)) {
	    T newCen = window.GetCenterAtom();
	    SetRebuildFlag(newCen);
	    window.SetCenterAtom(newCen);
	  }
        }
      }
    }
  }
}
