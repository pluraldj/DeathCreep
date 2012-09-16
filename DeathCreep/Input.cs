using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using libtcod;

namespace DeathCreep
{
    class Input
    {
        public bool handleKeys(Thing player, List<Tile> walls)
        {
            var key = TCODConsole.waitForKeypress(true);

            if (key.KeyCode == TCODKeyCode.Enter && key.KeyCode == TCODKeyCode.Alt)
            {
                TCODConsole.setFullscreen(true);
            }
            else if (key.KeyCode == TCODKeyCode.Escape)
            {
                return true;
            }

            if (TCODConsole.isKeyPressed(TCODKeyCode.Left))
            {
                if (walls.Count(tile => tile.tileX == player.x - 1 && tile.tileY == player.y && tile.blocked) == 0)
                {
                    player.move(-1, 0);
                }
            }
            else if (TCODConsole.isKeyPressed(TCODKeyCode.Right))
            {
                if (walls.Count(tile => tile.tileX == player.x + 1 && tile.tileY == player.y && tile.blocked) == 0)
                {
                    player.move(1, 0);
                }
            }
            else if (TCODConsole.isKeyPressed(TCODKeyCode.Up))
            {
                if (walls.Count(tile => tile.tileX == player.x && tile.tileY == player.y - 1 && tile.blocked) == 0)
                {
                    player.move(0, -1);
                }
            }
            else if (TCODConsole.isKeyPressed(TCODKeyCode.Down))
            {
                if (walls.Count(tile => tile.tileX == player.x && tile.tileY == player.y + 1 && tile.blocked) == 0)
                {
                    player.move(0, 1);
                }
            }

            return false;
        }
    }
}
