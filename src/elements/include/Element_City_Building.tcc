/* -*- C++ -*- */

#include "Element_City_Building.h"
#include "Element_City_Car.h"
#include "Element_City_Street.h"
#include "Element_City_Sidewalk.h"

namespace MFM
{
  template <class CC>
  u32 Element_City_Building<CC>::GetSidewalkType() const
  {
    return Element_City_Sidewalk<CC>::THE_INSTANCE.GetType();
  }

  template <class CC>
  u32 Element_City_Building<CC>::GetStreetType() const
  {
    return Element_City_Street<CC>::THE_INSTANCE.GetType();
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

  template <class CC>
  typename CC::ATOM_TYPE Element_City_Building<CC>::MakeRandomCar(u32 myBuildingType,
                                                                  Random& rand) const
  {
    s32 type;
    T car = Element_City_Car<CC>::THE_INSTANCE.GetDefaultAtom();
    do
    {
      type = rand.Create(CityConstants::CITY_BUILDING_COUNT);
    }while(type == myBuildingType);

    Element_City_Car<CC>::THE_INSTANCE.SetDestType(car, type);
    return car;
  }

  template <class CC>
  void Element_City_Building<CC>::SpawnCar(EventWindow<CC>& window) const
  {
    WindowScanner<CC> scanner(window);
    SPoint streetPt;

    if(scanner.FindRandomLocationOfType(GetStreetType(), streetPt))
    {
      /* Make a car with a random destination. Because we want
       * routing to be interesting, let's avoid spawning it with our
       * building type.*/
      T car = MakeRandomCar(GetSubType(window.GetCenterAtom()), window.GetRandom());

      Dir carDir = Element_City_Street<CC>::THE_INSTANCE.GetDirection(
                       window.GetRelativeAtom(streetPt));

      if(window.GetRandom().CreateBool())
      {
        carDir = Dirs::OppositeDir(carDir);
      }

      Element_City_Car<CC>::THE_INSTANCE.SetDirection(car, carDir);

      window.SetRelativeAtom(streetPt, car);
    }
  }
}
