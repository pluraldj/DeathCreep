using System;
using System.Collections.Generic;
using System.Linq;
using libtcod;

namespace DeathCreep
{
    public static class Program
    {
        public static void Main()
        {
            Thing player = new Thing(Console.screenX / 2, Console.screenY / 2, '@', TCODColor.white);
            Thing npc = new Thing(Console.screenX / 2 - 5, Console.screenY / 2, '@', TCODColor.yellow);

            List<Thing> things = new List<Thing>();
            things.Add(player);
            things.Add(npc);

            List<Tile> floor = new List<Tile>();
            for (int x = 0; x < Console.mapX; x++)
            {
                for (int y = 0; y < Console.mapY; y++)
                {
                    floor.Add(new Tile(x, y, false));
                }
            }

            List<Tile> walls = new List<Tile>();
            walls.Add(new Tile(50, 22, true));
            walls.Add(new Tile(30, 22, true));

            TCODConsole.initRoot(Console.screenX, Console.screenY, "DeathCreep", false);
            TCODSystem.setFps(25);

            TCODConsole.root.print(0, 0, "Welcome to DeathCreep! There's not much here.");
            TCODConsole.flush();

            bool endGame = false;

            while (!endGame && !TCODConsole.isWindowClosed())
            {
                RenderAll(things, floor, walls);
                Input move = new Input();
                endGame = move.handleKeys(player, walls);
            }
        }

        public static void RenderAll(List<Thing> things, List<Tile>floor, List<Tile>walls)
        {
            Map.MakeMap(floor);
            Map.MakeMap(walls);

            foreach (Thing t in things)
            {
                t.draw();
            }

            TCODConsole.flush();

            foreach (Thing t in things)
            {
                t.clear();
            }
        }
    }
}
