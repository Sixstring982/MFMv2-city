/*                                              -*- mode:C++ -*-
  Element_City_Building.h Vehicle spawn and consumpition point
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file Element_City_Building.h Vehicle spawn and consumpition point
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_CITY_BUILDING_H
#define ELEMENT_CITY_BUILDING_H

#include "CityConstants.h"
#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "Dirs.h"
#include "itype.h"
#include "P3Atom.h"

namespace MFM
{

#define BUILDING_VERSION 1

  template<class CC>
  class Element_City_Building : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum
    {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM,

      SUB_TYPE_POS = P3Atom<P>::P3_STATE_BITS_POS,
      SUB_TYPE_LEN = CityConstants::CITY_BUILDING_COUNT_LOG2,

      INITTED_POS = SUB_TYPE_POS + SUB_TYPE_LEN,
      INITTED_LEN = 1,

      AREA_INDEX_POS = INITTED_POS + INITTED_LEN,
      AREA_INDEX_LEN = 4,

      MAX_AREA_POS = AREA_INDEX_POS + AREA_INDEX_LEN,
      MAX_AREA_LEN = 6,

      MIN_AREA = 3
    };

    typedef BitField<BitVector<BITS>, VD::U32, SUB_TYPE_LEN, SUB_TYPE_POS> AFSubType;
    typedef BitField<BitVector<BITS>, VD::U32, INITTED_LEN, INITTED_POS> AFInitted;
    typedef BitField<BitVector<BITS>, VD::U32, MAX_AREA_LEN, MAX_AREA_POS> AFMaxArea;
    typedef BitField<BitVector<BITS>, VD::U32, AREA_INDEX_LEN, AREA_INDEX_POS> AFAreaIndex;

    ElementParameterS32<CC> m_carSpawnOdds;

   public:
    static Element_City_Building THE_INSTANCE;

    void SetAreaIndex(T& us, u32 val) const
    {
      AFAreaIndex::Write(this->GetBits(us), val);
    }

    u32 GetAreaIndex(const T& us) const
    {
      return AFAreaIndex::Read(this->GetBits(us));
    }

    void SetMaxArea(T& us, u32 val) const
    {
      AFMaxArea::Write(this->GetBits(us), val);
    }

    u32 GetMaxArea(const T& us) const
    {
      return AFMaxArea::Read(this->GetBits(us));
    }

    void Initialize(T& us, Random& rand) const
    {
      SetInitted(us, true);
      SetAreaIndex(us, 0);
      u32 maxArea = rand.Create(1 << MAX_AREA_LEN);
      if(maxArea < MIN_AREA)
      {
        maxArea = MIN_AREA;
      }
      SetMaxArea(us, maxArea);
      SetSubType(us, rand.Create(CityConstants::CITY_BUILDING_COUNT));
    }

    bool IsInitted(const T& us) const
    {
      return AFInitted::Read(this->GetBits(us));
    }

    void SetInitted(T& us, bool val) const
    {
      AFInitted::Write(this->GetBits(us), val);
    }

    u32 GetSubType(const T& us) const
    {
      return AFSubType::Read(this->GetBits(us));
    }
    void SetSubType(T& us, u32 val) const
    {
      AFSubType::Write(this->GetBits(us), val);
    }

    void SetRandSubtype(T& us, Random& rand) const
    {
      SetSubType(us, rand.Create(CityConstants::CITY_BUILDING_COUNT));
    }

    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    Element_City_Building() :
      Element<CC>(MFM_UUID_FOR("CityBuilding", BUILDING_VERSION)),
      m_carSpawnOdds(this, "carSpawnOdds", "Car Spawn Odds",
                     "Odds of a building spawning a car.", 1, 5000, 10000)
    {
      Element<CC>::SetAtomicSymbol("Bd");
      Element<CC>::SetName("City Building");
    }

    virtual const T& GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(), 0, 0, 0);

      return defaultAtom;
    }

    virtual u32 PercentMovable(const T& you, const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff800000;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff426344;
    }

    virtual u32 LocalPhysicsColor(const T& atom, u32 selector) const
    {
      switch(GetSubType(atom))
      {
      case 0x0: return 0xffff0000;
      case 0x1: return 0xff00ff00;
      case 0x2: return 0xff0000ff;
      case 0x3: return 0xff00ffff;
      case 0x4: return 0xffff00ff;
      case 0x5: return 0xffffff00;
      case 0x6: return 0xff7f0000;
      case 0x7: return 0xff007f00;
      case 0x8: return 0xff00007f;
      case 0x9: return 0xff007f7f;
      case 0xa: return 0xff7f007f;
      case 0xb: return 0xff7f7f00;
      case 0xc: return 0xffffffff;
      case 0xd: return 0xff400040;
      case 0xe: return 0xff7f7f7f;
      case 0xf: return 0xff404040;
      default:  return 0xff800000;
      }
    }

    virtual const char* GetDescription() const
    {
      return "City filler for surrounded sidewalks";
    }

    virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return nowAt.Equals(maybeAt)?Element<CC>::COMPLETE_DIFFUSABILITY:0;
    }

    u32 LargestVisibleIndex(EventWindow<CC>& window) const
    {
      MDist<R>& md = MDist<R>::get();
      u32 largestIdx = 0;
      for(u32 i = md.GetFirstIndex(0); i <= md.GetLastIndex(R); i++)
      {
        SPoint pt = md.GetPoint(i);
        if(window.GetRelativeAtom(pt).GetType() == TYPE())
        {
          if(GetSubType(window.GetRelativeAtom(pt)) ==
             GetSubType(window.GetCenterAtom()))
          {
            if(largestIdx < GetAreaIndex(window.GetCenterAtom()))
            {
              largestIdx = GetAreaIndex(window.GetCenterAtom());
            }
          }
        }
      }
      return largestIdx;
    }

    u32 GetSidewalkType() const;

    u32 GetStreetType() const;

    u32 GetIntersectionType() const;

    u32 GetCarType() const;

    bool DoNBSidewalkCase(EventWindow<CC>& window) const;

    bool DoNBPerpGrowthCase(EventWindow<CC>& window) const;

    bool DoNoSidewalkConsume(EventWindow<CC>& window) const;

    void SpawnNextBuilding(EventWindow<CC>& window) const;

    void SpawnNextBuildingSnakey(EventWindow<CC>& window) const
    {
      if(GetAreaIndex(window.GetCenterAtom()) < GetMaxArea(window.GetCenterAtom()))
      {
        MDist<R>& md = MDist<R>::get();
        bool spawnedNextBuilding = false;
        for(u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(R); i++)
        {
          SPoint pt = md.GetPoint(i);
          T atom;

          if((atom = window.GetRelativeAtom(pt)).GetType() == TYPE())
          {
            if(GetSubType(atom) == GetSubType(window.GetCenterAtom()))
            {
              if(GetAreaIndex(atom) == GetAreaIndex(window.GetCenterAtom()) + 1)
              {
                spawnedNextBuilding = true;
                break;
              }
            }
          }
        }

        if(!spawnedNextBuilding)
        {
          SPoint empty;
          u32 emptyCount = 0;
          for(u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(1); i++)
          {
            SPoint pt = md.GetPoint(i);
            if(window.GetRelativeAtom(pt).GetType() == Element_Empty<CC>::THE_INSTANCE.GetType())
            {
              if(window.GetRandom().OneIn(++emptyCount))
              {
                empty = pt;
              }
            }
          }

          if(emptyCount > 0)
          {
            T building = window.GetCenterAtom();
            SetAreaIndex(building, GetAreaIndex(building) + 1);
            window.SetRelativeAtom(empty, building);
          }
        }
      }
    }

    T MakeRandomCar(u32 myBuildingType, Random& rand) const;

    void SpawnCar(EventWindow<CC>& window) const;

   public:
    virtual void Behavior(EventWindow<CC>& window) const
    {
      if(!IsInitted(window.GetCenterAtom()))
      {
        T me = window.GetCenterAtom();
        Initialize(me, window.GetRandom());
        window.SetCenterAtom(me);
      }
      else
      {
        if(window.GetRandom().OneIn(m_carSpawnOdds.GetValue()))
        {
          SpawnCar(window);
        }

        SpawnNextBuilding(window);
      }
    }
  };

  template <class CC>
  Element_City_Building<CC> Element_City_Building<CC>::THE_INSTANCE;
}

#include "Element_City_Building.tcc"

#endif /* ELEMENT_CITY_BUILDING_H */
