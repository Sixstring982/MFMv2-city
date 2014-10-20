/* -*- C++ -*- */

#include "Element_City_Building.h"
#include "Element_City_Street.h"

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
  void Element_City_Car<CC>::Behavior(EventWindow<CC>& window) const
  {
    SPoint heading;
    Dirs::FillDir(heading, GetDirection(window.GetCenterAtom()));

    {
      T newMe = window.GetCenterAtom();
      if(UseGas(newMe))
      {
        window.SetCenterAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
        return;
      }
    }

    if(!window.IsLiveSite(heading))
    {
      /* Can't move there? Reset our heading and wait for another
       * event.*/
      T newMe = window.GetCenterAtom();
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
        if(window.GetRelativeAtom(buildings[i]).GetType() == GetBuildingType())
        {
          if(Element_City_Building<CC>::THE_INSTANCE.GetSubType(
                 window.GetRelativeAtom(buildings[i])) ==
             GetDestType(window.GetCenterAtom()))
          {
            /* Found a building! Finally. Report our gas usage. */
            LOG.Message("Gas Usage: %d", GetGas(window.GetCenterAtom()));
            window.SetCenterAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
            return;
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
