using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using libtcod;

namespace DeathCreep
{
    public class Thing
    {
        public int x;
        public int y;
        public int symbol;
        public TCODColor color;

        public Thing(int x, int y, int symbol, TCODColor color)
        {
            this.x = x;
            this.y = y;
            this.symbol = symbol;
            this.color = color;
        }

        public void move(int dx, int dy)
        {
            this.x += dx;
            this.y += dy;
        }

        public void draw()
        {
            TCODConsole.root.setForegroundColor(this.color);
            TCODConsole.root.putChar(this.x, this.y, this.symbol, TCODBackgroundFlag.None);
        }

        public void clear()
        {
            TCODConsole.root.putChar(this.x, this.y, ' ');
        }
    }
}
