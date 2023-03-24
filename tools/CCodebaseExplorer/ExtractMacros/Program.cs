using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

//extract macros on which the code base is relying on.
//#ifdef and #ifndef and #elif
namespace ExtractMacros
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

            HashSet<string> includes = new HashSet<string>();
            foreach (string pattern in new string[] { "*.c", "*.h" })
            {
                foreach (string file in Directory.EnumerateFiles(args[0], pattern, SearchOption.AllDirectories))
                {
                    foreach (string line in File.ReadAllLines(file))
                    {
                        if (Regex.IsMatch(line, "#.*?ifdef ") ||
                            Regex.IsMatch(line, "#.*?ifndef ") ||
                            Regex.IsMatch(line, "#.*?elif "))
                        {
                            if (!includes.Contains(line))
                                includes.Add(line.Trim());
                        }
                    }
                }
            }
            foreach (string include in includes)
            {
                Console.WriteLine(include);
            }
        }
    }
}
