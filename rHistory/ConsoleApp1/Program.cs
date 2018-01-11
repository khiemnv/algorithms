using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ConsoleApp1
{
    class rHistory
    {
        const int maxcount = 2;
        int nCount;
        int ifirst;
        int cursor;
        string[] data;
        public rHistory()
        {
            nCount = 0;
            ifirst = 0;
            cursor = 0; //zero base
            data = new string[maxcount];
        }
        public void add(string txt)
        {
            nCount = cursor + 1;
            if (nCount == maxcount)
            {
                ifirst = (ifirst + 1) % maxcount;
            }
            else
                nCount++;

            cursor = nCount - 1;
            var i = (ifirst + cursor) % maxcount;
            data[i] = txt;
        }
        //[cursor][next]
        public string next()
        {
            var ret = "";
            if (cursor < (nCount - 1))
            {
                cursor++;
                var i = (cursor + ifirst) % maxcount;
                ret = data[i];
            }
            return ret;
        }
        //[prev][cursor]
        public string prev()
        {
            var ret = "";
            if (cursor > 0)
            {
                cursor--;
                var i = (cursor + ifirst) % maxcount;
                ret = data[i];
            }
            return ret;
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            rHistory obj = new rHistory();
            obj.add("1");
            obj.add("2");
            obj.add("3");
            Debug.Assert(obj.next() == "");
            Debug.Assert(obj.prev() == "2");
            Debug.Assert(obj.prev() == "");
            Debug.Assert(obj.next() == "3");
            Debug.Assert(obj.next() == "");

            obj.add("4");
            obj.add("5");
            Debug.Assert(obj.prev() == "4");
            obj.add("6");
            Debug.Assert(obj.prev() == "4");
            Debug.Assert(obj.next() == "6");
            obj.add("7");
            Debug.Assert(obj.prev() == "6");
            Debug.Assert(obj.prev() == "");
        }
    }
}

