using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using libtcod;

namespace DeathCreep
{
    public class Tile
    {
        public bool blocked;
        public bool block_sight;
        public int tileX;
        public int tileY;

        public Tile(int tileX, int tileY, bool blocked, bool block_sight = false)
        {
            this.blocked = blocked;
            this.tileX = tileX;
            this.tileY = tileY;

            if (!block_sight)
            {
                this.block_sight = blocked;
            }
            else
            {
                this.block_sight = block_sight;
            }
        }

        public Tile Map(int x, int y)
        {
            List<Tile> floor = new List<Tile>();
            Tile newTile = new Tile(x, y, false);

            for (int mx = 0; mx < Console.mapX; mx++)
            {
                for (int my = 0; my < Console.mapY; my++)
                {
                    floor.Add(new Tile(mx, my, false));
                }
            }

            foreach (Tile t in floor)
            {
                if (t.blocked)
                {
                    TCODConsole.root.setCharBackground(t.tileX, t.tileY, Console.colorDarkWall);
                }
                else
                {
                    TCODConsole.root.setCharBackground(t.tileX, t.tileY, Console.colorDarkGround);
                }

                newTile = t;
            }

            return newTile;
        }
    }
}
