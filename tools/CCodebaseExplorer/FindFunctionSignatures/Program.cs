using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FindFunctionSignatures
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

            foreach (string pattern in new string[] { "*.c", "*.h" })
            {
                foreach (string file in Directory.EnumerateFiles(args[0], pattern, SearchOption.AllDirectories))
                {
                    ProcessFile(file);
                }
            }
        }

        private static void ProcessFile(string file)
        {
            Console.WriteLine("========================================");
            Console.WriteLine(file);
            bool PrintRemaining = false;
            string prev = "";
            foreach(string _line in File.ReadAllLines(file))
            {
                string line = _line;
                if (PrintRemaining)
                {
                    Console.WriteLine(line);
                    if (line.Contains(")"))
                    {
                        PrintRemaining = false;
                    }
                }
                else if (line.Length > 0 && Char.IsLetter(line[0]))
                {
                    if (line.Contains("("))
                    {
                        if (prev.Trim().Length > 0)
                            Console.WriteLine(prev);
                        Console.WriteLine(line);
                        if (!line.Contains(")"))
                            PrintRemaining = true;
                    }
                }
                prev = line;
            }
        }
    }
}
