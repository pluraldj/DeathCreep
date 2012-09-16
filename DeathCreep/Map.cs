using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using libtcod;

namespace DeathCreep
{
    public static class Map
    {
        public static void MakeMap(List<Tile> tiles)
        {
            foreach (Tile t in tiles)
            {
                if (t.blocked)
                {
                    TCODConsole.root.setCharBackground(t.tileX, t.tileY, Console.colorDarkWall);
                }
                else
                {
                    TCODConsole.root.setCharBackground(t.tileX, t.tileY, Console.colorDarkGround);
                }
            }
        }
    }
}
