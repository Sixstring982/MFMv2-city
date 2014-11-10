/* -*- C++ -*- */

#include "Element_City_Building.h"
#include "Element_City_Street.h"
#include "Element_City_Intersection.h"

namespace MFM
{

  template <class CC>
  u32 Element_City_Car<CC>::GetBuildingType() const
  {
    return Element_City_Building<CC>::THE_INSTANCE.GetType();
  }

  template <class CC>
  u32 Element_City_Car<CC>::GetStreetType() const
  {
    return Element_City_Street<CC>::THE_INSTANCE.GetType();
  }

  template <class CC>
  u32 Element_City_Car<CC>::GetSidewalkType() const
  {
    return Element_City_Sidewalk<CC>::THE_INSTANCE.GetType();
  }

  template <class CC>
  u32 Element_City_Car<CC>::GetIntersectionType() const
  {
    return Element_City_Intersection<CC>::THE_INSTANCE.GetType();
  }

  template <class CC>
  void Element_City_Car<CC>::ReplaceCenterWithStreet(EventWindow<CC>& window) const
  {
    T newStreet = Element_City_Street<CC>::THE_INSTANCE.GetDefaultAtom();
    Element_City_Street<CC>::THE_INSTANCE.
    SetDirection(newStreet,
                 GetDirection(window.GetCenterAtom()));
    window.SetCenterAtom(newStreet);
  }

  template <class CC>
  void Element_City_Car<CC>::Behavior(EventWindow<CC>& window) const
  {
    SPoint heading;
    T newMe;
    WindowScanner<CC> scanner(window);
    Dirs::FillDir(heading, GetDirection(window.GetCenterAtom()));

    newMe = window.GetCenterAtom();

    if(UseGas(newMe))
    {
      ReplaceCenterWithStreet(window);
      LOG.Message("Out of gas!");
      return;
    }
    else
    {
      window.SetCenterAtom(newMe);
    }

    if(!window.IsLiveSite(heading) ||
       window.GetRelativeAtom(heading).GetType() == GetSidewalkType())
    {
      /* Can't move there? Turn around and wait for another
       * event.*/
      SetDirection(newMe, Dirs::OppositeDir(GetDirection(newMe)));
      window.SetCenterAtom(newMe);
    }
    else
    {
      SPoint buildings[2];
      Dirs::FillDir(buildings[0],
                    Dirs::CWDir(Dirs::CWDir(GetDirection(window.GetCenterAtom()))));
      Dirs::FillDir(buildings[1],
                    Dirs::CCWDir(Dirs::CCWDir(GetDirection(window.GetCenterAtom()))));

      for(u32 i = 0; i < 2; i++)
      {
        buildings[i] *= 2; /* Was pointing at sidewalk */
        if(window.GetRelativeAtom(buildings[i]).GetType() == GetBuildingType())
        {
          if(Element_City_Building<CC>::THE_INSTANCE.GetSubType(
                 window.GetRelativeAtom(buildings[i])) ==
             GetDestType(window.GetCenterAtom()))
          {
            /* Found a building! Finally. Report our gas usage. */
            LOG.Message("Gas Usage: %d", GetGas(window.GetCenterAtom()));
            ReplaceCenterWithStreet(window);
            return;
          }
        }
      }

      /* What if I'm at an intersection and the intersection borders
         the  building I want to be at? Let's get consumed there too. */

      SPoint intersectionPt;
      if(scanner.FindRandomInVonNeumann(GetIntersectionType(),
                                        intersectionPt) > 0)
      {
        MDist<R>& md = MDist<R>::get();
        for(u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(1); i++)
        {
          SPoint mdp = (md.GetPoint(i) * 2) + intersectionPt;
          if(window.GetRelativeAtom(mdp).GetType() == GetBuildingType())
          {
            if(Element_City_Building<CC>::THE_INSTANCE.GetSubType(
                                          window.GetRelativeAtom(mdp)) ==
               GetDestType(window.GetCenterAtom()))
            {
              /* Found a building! Finally. Report our gas usage. */
              LOG.Message("Gas Usage: %d", GetGas(window.GetCenterAtom()));
              ReplaceCenterWithStreet(window);
              return;
            }
          }
        }
      }

      if(window.GetRelativeAtom(heading).GetType() == TYPE() ||
         window.GetRelativeAtom(heading).GetType() == GetStreetType())
      {
        /* We can move! swap. */
        window.SwapAtoms(SPoint(0, 0), heading);
      }
    }
  }
}
