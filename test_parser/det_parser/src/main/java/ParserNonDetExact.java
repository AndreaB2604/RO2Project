import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import java.util.*;
import java.io.*;

public class ParserNonDetExact
{
	private final String DIRECTORY_PATH = "src/main/resources/allNonDet";
	List<Instance> firstTableList = new ArrayList<>();
	List<Instance> secondTableList = new ArrayList<>();

	public static void main(String[] args) throws IOException
	{
		(new ParserNonDetExact()).exec(args);
	}
	
	void exec(String[] args) throws IOException
	{
		File folder = new File(DIRECTORY_PATH);

		File[] listFiles = folder.listFiles();
		Arrays.sort(listFiles);
		//int i = 0;
		for(File file : listFiles) 
		{
			if(!file.isDirectory())
			{
				String fileName = file.getName();
				if(!fileName.contains("tsp")) 
					continue;
				
				//System.out.println(fileName);

				Instance instance = new Instance();
				instance.setFile(file);
				
				String name = fileName.split("\\.")[0];
				instance.setName(name);
				
				String method = fileName.split("-")[1];
				instance.setMethod(method);

				long randomSeed = Long.parseLong(fileName.split("-")[2]);
				instance.setRandomSeed(randomSeed);
 			
 				String isOpt = "";
 				double second = 3600.0;

 				if(isEmpty(instance.getFile()))
 				{
 					isOpt = "no";
 					second = 3600.0;
 				}
 				else
 				{
 					Scanner sc = new Scanner(instance.getFile());
 					while(sc.hasNextLine())
 					{
 						String line = sc.nextLine();
 						//System.out.println(line);
 						if(line.startsWith("Solved"))
 						{
 							isOpt = (line.split(" ")[1].equals("1"))? "yes" : "no";
 							sc.nextLine();
 							second = Double.parseDouble(sc.nextLine().split(" ")[5]);
 							break;
 						}
 					}
 				}

				if(fileName.split("-")[3].equals("1"))
				{
					instance.setAvgMean(second/3);
 					instance.setGeoMean(Math.pow(second, (1/3.0)));
 					instance.setIsOpt(isOpt);
 					//System.out.println("inserisco");
					if(randomSeed < 40000)
						firstTableList.add(instance);
					else
						secondTableList.add(instance);
				}
				else
				{
					List<Instance> listToCheck;
					if(randomSeed < 40000)
						listToCheck = firstTableList;
					else
						listToCheck = secondTableList;

					for(Instance instInside : listToCheck)
					{
						if(instInside.compareTo(instance) == 0)
						{
							//System.out.println("Aggiorno");
							instInside.setAvgMean(instInside.getAvgMean() + second/3);
 							instInside.setGeoMean(instInside.getGeoMean() * Math.pow(second, (1/3.0)));
 							if(isOpt.equals("yes"))
 								instInside.setIsOpt(isOpt);
						}
					}
				}
				//if(i++ == 18)
				//	break;
			}
		}

		printResults("sec_loop", 1);
		printResults("sec_loop", 2);

		printResults("sec_callback", 1);
		printResults("sec_callback", 2);

		printResults("usr_callback", 1);
		printResults("usr_callback", 2);


		System.out.println("\n");
		
		System.out.println("Number of files = " + listFiles.length);

	}

	private void printResults(String method, int type)
	{
		if(type == 1)
		{
			System.out.println("\nTABELLA 1 ESATTI METODO " + method);
			String first = "";
			String second = "";
			String third = "";
			for(Instance inst : firstTableList)
			{
				if(inst.getMethod().equals(method))
				{
					if(inst.getRandomSeed() == 12)
					{
						first = "\\midrule " 
								+ inst.getName() + " & " 
								+ String.format(Locale.ENGLISH, "%.2f", inst.getAvgMean()) + " & " 
								+ String.format(Locale.ENGLISH, "%.2f", inst.getGeoMean()) + " & " 
								+ inst.getIsOpt();
					}
					else if(inst.getRandomSeed() == 893)
					{
						second = " & " 
								+ String.format(Locale.ENGLISH, "%.2f", inst.getAvgMean()) + " & " 
								+ String.format(Locale.ENGLISH, "%.2f", inst.getGeoMean()) + " & " 
								+ inst.getIsOpt();
						System.out.println(first + second + third);
						first = "";
						second = "";
						third = "";
					}
					else if(inst.getRandomSeed() == 39848)
					{
						third = " & " 
								+ String.format(Locale.ENGLISH, "%.2f", inst.getAvgMean()) + " & " 
								+ String.format(Locale.ENGLISH, "%.2f", inst.getGeoMean()) + " & " 
								+ inst.getIsOpt() + " \\\\";
					}
				}
			}
		}
		else if(type == 2)
		{
			System.out.println("\nTABELLA 2 ESATTI METODO " + method);
			String first = "";
			String second = "";
			String third = "";
			for(Instance inst : secondTableList)
			{
				if(inst.getMethod().equals(method))
				{
					if(inst.getRandomSeed() == 2648948)
						first = "\\midrule " 
								+ inst.getName() + " & " 
								+ String.format(Locale.ENGLISH, "%.2f", inst.getAvgMean()) + " & " 
								+ String.format(Locale.ENGLISH, "%.2f", inst.getGeoMean()) + " & " 
								+ inst.getIsOpt();
					else if(inst.getRandomSeed() == 201709013)
					{
						second = " & " 
								+ String.format(Locale.ENGLISH, "%.2f", inst.getAvgMean()) + " & " 
								+ String.format(Locale.ENGLISH, "%.2f", inst.getGeoMean()) + " & " 
								+ inst.getIsOpt();
					}
					else if(inst.getRandomSeed() == 9837745292L)
					{
						third = " & " 
								+ String.format(Locale.ENGLISH, "%.2f", inst.getAvgMean()) + " & " 
								+ String.format(Locale.ENGLISH, "%.2f", inst.getGeoMean()) + " & " 
								+ inst.getIsOpt() + " \\\\";
						System.out.println(first + second + third);
						first = "";
						second = "";
						third = "";
					}
				}
			}
		}
	}

	public static boolean isEmpty(File file) throws IOException
	{
		BufferedReader br = new BufferedReader(new FileReader(file));
		return (br.readLine() == null)? true : false;
	}


	public static void parseHTML(Map<String, Long> tMap)
	{
		final String PATH = "src/main/resources/optSol.html";
		try
		{
			File file = new File(PATH);
			
			Document doc = Jsoup.parse(file, "UTF-8");
			
			Elements newsHeadlines = doc.select("big");
			
			String trash = null;
			String container = null;
			for(Element headline : newsHeadlines)
			{
				String opt = headline.text();
				String[] strArray = opt.split(" : ");
				if(opt.contains(":"))
				{
					tMap.put(strArray[0], Long.parseLong(strArray[1].split(" ")[0]));
				}
			}
		} 
		catch (IOException ioe)
		{
			ioe.printStackTrace();
		}
	}
}