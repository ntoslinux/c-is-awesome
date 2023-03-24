using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace ExtractStrutures
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("pass directory as argument");
                return;
            }
            //HashSet<String> structs = new HashSet<string>();
            foreach (string pattern in new string[] { "*.c", "*.h" })
            {
                foreach (string file in Directory.EnumerateFiles(args[0], pattern, SearchOption.AllDirectories))
                {
                    string[] lines = File.ReadAllLines(file);
                    for (int i = 0; i < lines.Length;  )
                    {
                        Match m = Regex.Match(lines[i], "^struct (.*?) {");
                        if (m.Success)
                        {
                            //if (structs.Contains(m.Groups[0].Value))
                            //{
                            //    i++;
                            //    continue;
                            //}

                            //structs.Add(m.Groups[0].Value);
                            Console.WriteLine(file);
                            ExtractStucture(lines, ref i);
                        }
                        else i++;
                    }
                }
            }

        }

        private static void ExtractStucture(string[] Lines, ref int LineNum)
        {
            int i = 0;
            for (i = LineNum; !Lines[i].StartsWith("}"); i++)
                Console.WriteLine(Lines[i]);
            Console.WriteLine(Lines[i]);
            LineNum = i + 1;
        }
    }
}
