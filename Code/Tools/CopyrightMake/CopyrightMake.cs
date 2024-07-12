using CommandLine;
using System;
using System.IO;
using System.Text;
namespace CopyrightMake
{
  public class WDCopyrightMakeOptions
  {

    [Option('s', "source", Required = true, HelpText = "Source directory where the engine is stored")]
    public string rootDirectory { get; set; }

    [Option('o', "output", Required = true, HelpText = "Output license notices file name", Default = "thirdpartylegalnotices.txt")]
    public string outputFilePath { get; set; }
  }


  class LicenseCopier
  {
    public WDCopyrightMakeOptions options;
    static void Main(string[] args)
    {
      string rootDirectory = @"C:\Path\To\Your\Directory";
      string outputFilePath = Path.Combine(rootDirectory, "thirdpartylicenses.txt");
      Console.WriteLine("CopyrightMake. Copyright (C) 2024 Mikael K. Aboagye. All Rights reserved. this project is licensed under the MIT License.");
      Parser.Default.ParseArguments<WDCopyrightMakeOptions>(args)
                        .WithParsed<WDCopyrightMakeOptions>(o =>
                        {
                          if (o.rootDirectory != null && o.outputFilePath != null)
                          {
                            Console.WriteLine($"Current Arguments: -s {o.rootDirectory}, -o {o.outputFilePath}");
                            rootDirectory = o.rootDirectory;
                            outputFilePath = Path.Combine(o.rootDirectory, o.outputFilePath);
                          }
                          else if (o.outputFilePath == null || o.rootDirectory == null)
                          {
                            Console.WriteLine("one of the params is null.");
                          }
                          else
                          {
                            Console.WriteLine("Project Name is null. please provide a project name.");
                          }
                        });


      try
      {
        using (StreamWriter outputFile = new StreamWriter(outputFilePath, false, Encoding.UTF8))
        {
          SearchAndCopyLicenses(rootDirectory, outputFile);
        }

        Console.WriteLine($"Licenses copied successfully to: {outputFilePath}");
      }
      catch (Exception ex)
      {
        Console.WriteLine($"An error occurred: {ex.Message}");
      }
    }

    static void SearchAndCopyLicenses(string directory, StreamWriter outputFile)
    {
      try
      {
        // Get all files with "LICENSE" in their name
        string[] licenseFiles = Directory.GetFiles(directory, "*LICENSE.*", SearchOption.AllDirectories);

        foreach (string licenseFile in licenseFiles)
        {
          // Get the license name from the file name
          //string licenseName = Path.GetFileNameWithoutExtension(licenseFile);
          string licenseName = Path.GetDirectoryName($"{licenseFile}");
          // Write the license name to the output file
          outputFile.WriteLine(new string('=', 70));
          outputFile.WriteLine($"{licenseName} (LICENSE)");
          outputFile.WriteLine(new string('=', 70));

          // Copy the contents of the license file
          string licenseContent = File.ReadAllText(licenseFile);
          outputFile.WriteLine(licenseContent);
          outputFile.WriteLine();
        }
      }
      catch (Exception ex)
      {
        Console.WriteLine($"An error occurred while processing licenses: {ex.Message}");
      }
    }
  }

}
