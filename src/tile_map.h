/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tile_map.h Map writing/reading functions for tiles. */

#ifndef TILE_MAP_H
#define TILE_MAP_H

#include "slope_type.h"
#include "map_func.h"
#include "core/bitmath_func.hpp"
#include "settings_type.h"

/**
 * Returns the height of a tile
 *
 * This function returns the height of the northern corner of a tile.
 * This is saved in the global map-array. It does not take affect by
 * any slope-data of the tile.
 *
 * @param tile The tile to get the height from
 * @return the height of the tile
 * @pre tile < MapSize()
 */
static inline uint TileHeight(TileIndex tile)
{
	assert(tile < MapSize());
	return GB(_mth[tile].type_height, 0, 4);
}

/**
 * Sets the height of a tile.
 *
 * This function sets the height of the northern corner of a tile.
 *
 * @param tile The tile to change the height
 * @param height The new height value of the tile
 * @pre tile < MapSize()
 * @pre heigth <= MAX_TILE_HEIGHT
 */
static inline void SetTileHeight(TileIndex tile, uint height)
{
	assert(tile < MapSize());
	assert(height <= MAX_TILE_HEIGHT);
	SB(_mth[tile].type_height, 0, 4, height);
}

/**
 * Returns the height of a tile in pixels.
 *
 * This function returns the height of the northern corner of a tile in pixels.
 *
 * @param tile The tile to get the height
 * @return The height of the tile in pixel
 */
static inline uint TilePixelHeight(TileIndex tile)
{
	return TileHeight(tile) * TILE_HEIGHT;
}

/**
 * Get the tiletype of a given tile.
 *
 * @param tile The tile to get the TileType
 * @return The tiletype of the tile
 * @pre tile < MapSize()
 */
static inline TileType GetTileType(TileIndex tile)
{
	assert(tile < MapSize());
	uint type = GB(_mc[tile].m0, 4, 4);
	return (TileType)type;
}

/**
 * Check if a tile is within the map (not a border)
 *
 * @param tile The tile to check
 * @return Whether the tile is in the interior of the map
 * @pre tile < MapSize()
 */
static inline bool IsInnerTile(TileIndex tile)
{
	assert(tile < MapSize());

	uint x = TileX(tile);
	uint y = TileY(tile);

	return x < MapMaxX() && y < MapMaxY() && ((x > 0 && y > 0) || !_settings_game.construction.freeform_edges);
}

/**
 * Set the type of a tile
 *
 * This functions sets the type of a tile. At the south-west or
 * south-east edges of the map, only void tiles are allowed.
 *
 * @param tile The tile to save the new type
 * @param type The type to save
 * @pre tile < MapSize()
 * @pre void type <=> tile is on the south-east or south-west edge.
 */
static inline void SetTileType(TileIndex tile, TileType type)
{
	assert(tile < MapSize());
	assert(type < 12);
	/* Only void tiles are allowed at the lower left and right
	 * edges of the map. If _settings_game.construction.freeform_edges is true,
	 * the upper edges of the map are also VOID tiles. */
	assert(IsInnerTile(tile) || (type == TT_VOID_TEMP));
	SB(_mc[tile].m0, 4, 4, type);
}

/**
 * Get the tile subtype of a given tile.
 *
 * @param tile The tile to get the TileSubtype
 * @return The TileSubtype of the tile
 * @pre tile < MapSize() and tile type has subtypes
 */
static inline TileSubtype GetTileSubtype(TileIndex tile)
{
	assert(tile < MapSize());
	assert(TileTypeHasSubtypes(GetTileType(tile)));
	uint subtype = GB(_mc[tile].m1, 6, 2);
	return (TileSubtype)subtype;
}

/**
 * Set the type and subtype of a tile.
 * @param tile The tile to set
 * @param type The type to set
 * @param subtype The subtype to set
 * @pre tile < MapSize() and tile type has subtypes
 */
static inline void SetTileTypeSubtype(TileIndex tile, TileType type, TileSubtype subtype)
{
	assert(tile < MapSize());
	assert(type < 8);
	assert(TileTypeHasSubtypes(type));
	SB(_mc[tile].m0, 4, 4, type);
	SB(_mc[tile].m1, 6, 2, subtype);
}

/**
 * Checks if a tile is a give tiletype.
 *
 * This function checks if a tile got the given tiletype.
 *
 * @param tile The tile to check
 * @param type The type to check against
 * @return true If the type matches against the type of the tile
 */
static inline bool IsTileType(TileIndex tile, TileType type)
{
	return GetTileType(tile) == type;
}

/**
 * Checks if a tile has a given subtype.
 *
 * @param tile The tile to check
 * @param subtype The subtype to check against
 * @return whether the tile has the given subtype
 * @note there is no check to ensure that the given subtype is allowed by the tile's type
 */
static inline bool IsTileSubtype(TileIndex tile, TileSubtype subtype)
{
	return GetTileSubtype(tile) == subtype;
}

/**
 * Checks if a tile has given type and subtype.
 *
 * @param tile The tile to check
 * @param type The type to check against
 * @param subtype The subtype to check against
 * @return whether the tile has the given type and subtype
 */
static inline bool IsTileTypeSubtype(TileIndex tile, TileType type, TileSubtype subtype)
{
	assert(TileTypeHasSubtypes(type));
	return IsTileType(tile, type) && IsTileSubtype(tile, subtype);
}

/**
 * Checks if a tile is clear.
 *
 * @param tile The tile to check
 * @return true If the tile is clear
 */
static inline bool IsClearTile(TileIndex tile)
{
	return IsTileType(tile, TT_GROUND);
}

/**
 * Checks if a tile is railway.
 *
 * @param tile The tile to check
 * @return true If the tile is railway
 */
static inline bool IsRailwayOrDepotTile(TileIndex tile)
{
	return IsTileType(tile, TT_RAILWAY);
}

/**
 * Checks if a tile has a road.
 *
 * @param tile The tile to check
 * @return true If the tile has a road
 */
static inline bool IsRoadOrDepotTile(TileIndex tile)
{
	return IsTileType(tile, TT_ROAD);
}

/**
 * Checks if a tile is a house.
 *
 * @param tile The tile to check
 * @return true If the tile is a house
 */
static inline bool IsHouseTile(TileIndex tile)
{
	return GB(_mc[tile].m0, 6, 2) == 3;
}

/**
 * Checks if a tile has trees.
 *
 * @param tile The tile to check
 * @return true If the tile has trees
 */
static inline bool IsTreeTile(TileIndex tile)
{
	return IsTileType(tile, TT_TREES_TEMP);
}

/**
 * Checks if a tile is a station tile.
 *
 * @param tile The tile to check
 * @return true If the tile is a station tile
 */
static inline bool IsStationTile(TileIndex tile)
{
	return IsTileType(tile, TT_STATION);
}

/**
 * Checks if a tile has water.
 *
 * @param tile The tile to check
 * @return true If the tile has water
 */
static inline bool IsWaterTile(TileIndex tile)
{
	return IsTileType(tile, TT_WATER);
}

/**
 * Checks if a tile is void.
 *
 * @param tile The tile to check
 * @return true If the tile is void
 */
static inline bool IsVoidTile(TileIndex tile)
{
	return IsTileType(tile, TT_VOID_TEMP);
}

/**
 * Checks if a tile is an industry.
 *
 * @param tile The tile to check
 * @return true If the tile is an industry
 */
static inline bool IsIndustryTile(TileIndex tile)
{
	return IsTileType(tile, TT_INDUSTRY_TEMP);
}

/**
 * Checks if a tile is a tunnel or bridge.
 *
 * @param tile The tile to check
 * @return true If the tile is a tunnel or bridge
 */
static inline bool IsTunnelBridgeTile(TileIndex tile)
{
	return IsTileType(tile, TT_TUNNELBRIDGE_TEMP);
}

/**
 * Checks if a tile has an object.
 *
 * @param tile The tile to check
 * @return true If the tile has an object
 */
static inline bool IsObjectTile(TileIndex tile)
{
	return IsTileType(tile, TT_OBJECT);
}

/**
 * Checks if a tile is a ground tile.
 *
 * @param tile The tile to check
 * @return true If the tile is a ground tile
 */
static inline bool IsGroundTile(TileIndex tile)
{
	return IsClearTile(tile) || IsTreeTile(tile);
}

/**
 * Checks if a tile is valid
 *
 * @param tile The tile to check
 * @return True if the tile is on the map and not void.
 */
static inline bool IsValidTile(TileIndex tile)
{
	return tile < MapSize() && !IsVoidTile(tile);
}

/**
 * Returns the owner of a tile
 *
 * This function returns the owner of a tile. This cannot used
 * for tiles whose type is one of void, house or industry,
 * as no company owned any of these buildings.
 *
 * @param tile The tile to check
 * @return The owner of the tile
 * @pre IsValidTile(tile)
 * @pre The tile must not be a house, an industry or void
 */
static inline Owner GetTileOwner(TileIndex tile)
{
	assert(IsValidTile(tile));
	assert(!IsHouseTile(tile));
	assert(!IsIndustryTile(tile));

	return (Owner)GB(_mc[tile].m1, 0, 5);
}

/**
 * Sets the owner of a tile
 *
 * This function sets the owner status of a tile. Note that you cannot
 * set a owner for tiles of type house, void or industry.
 *
 * @param tile The tile to change the owner status.
 * @param owner The new owner.
 * @pre IsValidTile(tile)
 * @pre The tile must not be a house, an industry or void
 */
static inline void SetTileOwner(TileIndex tile, Owner owner)
{
	assert(IsValidTile(tile));
	assert(!IsHouseTile(tile));
	assert(!IsIndustryTile(tile));

	SB(_mc[tile].m1, 0, 5, owner);
}

/**
 * Checks if a tile belongs to the given owner
 *
 * @param tile The tile to check
 * @param owner The owner to check against
 * @return True if a tile belongs the the given owner
 */
static inline bool IsTileOwner(TileIndex tile, Owner owner)
{
	return GetTileOwner(tile) == owner;
}

/**
 * Set the tropic zone
 * @param tile the tile to set the zone of
 * @param type the new type
 * @pre tile < MapSize()
 */
static inline void SetTropicZone(TileIndex tile, TropicZone type)
{
	assert(tile < MapSize());
	assert(!IsVoidTile(tile) || type == TROPICZONE_NORMAL);
	SB(_mth[tile].type_height, 6, 2, type);
}

/**
 * Get the tropic zone
 * @param tile the tile to get the zone of
 * @pre tile < MapSize()
 * @return the zone type
 */
static inline TropicZone GetTropicZone(TileIndex tile)
{
	assert(tile < MapSize());
	return (TropicZone)GB(_mth[tile].type_height, 6, 2);
}

/**
 * Get the current animation frame
 * @param t the tile
 * @pre IsHouseTile(t) || IsObjectTile(t) || IsIndustryTile(t) || IsStationTile(t)
 * @return frame number
 */
static inline byte GetAnimationFrame(TileIndex t)
{
	assert(IsHouseTile(t) || IsObjectTile(t) || IsIndustryTile(t) || IsStationTile(t));
	return _mc[t].m7;
}

/**
 * Set a new animation frame
 * @param t the tile
 * @param frame the new frame number
 * @pre IsHouseTile(t) || IsObjectTile(t) || IsIndustryTile(t) || IsStationTile(t)
 */
static inline void SetAnimationFrame(TileIndex t, byte frame)
{
	assert(IsHouseTile(t) || IsObjectTile(t) || IsIndustryTile(t) || IsStationTile(t));
	_mc[t].m7 = frame;
}

Slope GetTileSlope(TileIndex tile, int *h = NULL);
int GetTileZ(TileIndex tile);
int GetTileMaxZ(TileIndex tile);

bool IsTileFlat(TileIndex tile, int *h = NULL);

/**
 * Return the slope of a given tile
 * @param tile Tile to compute slope of
 * @param h    If not \c NULL, pointer to storage of z height
 * @return Slope of the tile, except for the HALFTILE part
 */
static inline Slope GetTilePixelSlope(TileIndex tile, int *h)
{
	Slope s = GetTileSlope(tile, h);
	if (h != NULL) *h *= TILE_HEIGHT;
	return s;
}

/**
 * Get bottom height of the tile
 * @param tile Tile to compute height of
 * @return Minimum height of the tile
 */
static inline int GetTilePixelZ(TileIndex tile)
{
	return GetTileZ(tile) * TILE_HEIGHT;
}

/**
 * Get top height of the tile
 * @param t Tile to compute height of
 * @return Maximum height of the tile
 */
static inline int GetTileMaxPixelZ(TileIndex tile)
{
	return GetTileMaxZ(tile) * TILE_HEIGHT;
}


/**
 * Compute the distance from a tile edge
 * @param side Tile edge
 * @param x x within the tile
 * @param y y within the tile
 * @return The distance from the edge
 */
static inline uint DistanceFromTileEdge(DiagDirection side, uint x, uint y)
{
	assert(x < TILE_SIZE);
	assert(y < TILE_SIZE);

	switch (side) {
		default: NOT_REACHED();
		case DIAGDIR_NE: return x;
		case DIAGDIR_SE: return TILE_SIZE - 1 - y;
		case DIAGDIR_SW: return TILE_SIZE - 1 - x;
		case DIAGDIR_NW: return y;
	}
}


/**
 * Calculate a hash value from a tile position
 *
 * @param x The X coordinate
 * @param y The Y coordinate
 * @return The hash of the tile
 */
static inline uint TileHash(uint x, uint y)
{
	uint hash = x >> 4;
	hash ^= x >> 6;
	hash ^= y >> 4;
	hash -= y >> 6;
	return hash;
}

/**
 * Get the last two bits of the TileHash
 *  from a tile position.
 *
 * @see TileHash()
 * @param x The X coordinate
 * @param y The Y coordinate
 * @return The last two bits from hash of the tile
 */
static inline uint TileHash2Bit(uint x, uint y)
{
	return GB(TileHash(x, y), 0, 2);
}

#endif /* TILE_MAP_H */
