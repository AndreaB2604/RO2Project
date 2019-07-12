import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import java.util.*;
import java.io.*;

public class ParserModulo
{
	public static void main(String[] args)
	{
		(new ParserModulo()).exec(args);
	}
	
	void exec(String[] args)
	{
		File folder = new File("src/main/resources/modulo");

		Map<String, Long> optValueMap = new TreeMap<>(); 
		parseHTML(optValueMap);

		File[] listFiles = folder.listFiles();
		Arrays.sort(listFiles);
		List<Instance> moduloList = new ArrayList<>();
		for(File file : listFiles) 
		{
			if(!file.isDirectory())
			{
				Instance instance = new Instance();
				instance.setFile(file);
				
				String fileName = file.getName();
				
				if(!fileName.contains("tsp"))
					continue;

				String name = fileName.split("\\.")[0];
				instance.setName(name);
				instance.setOptValue(optValueMap.get(name));

				String method = fileName.split("-")[1];
				instance.setMethod(method);
				
				moduloList.add(instance);
			}
		}

		System.out.println("TABELLA MODULO");
		for(Instance inst : moduloList)
		{
			Scanner sc = null;
			try
			{
				sc = new Scanner(inst.getFile());
				
				long afterVNS = -1;
				long afterHF = -1;
				while(sc.hasNextLine())
				{
					String line = sc.nextLine();
					//System.out.println(line);
					if(line.startsWith("solution value at"))
					{
						afterVNS = (long) Double.parseDouble(line.split(" ")[5]);
					}
					if(line.startsWith("improved value at"))
					{
						afterHF = (long) Double.parseDouble(line.split(" ")[5]);
					}
				}
				double gap = gapPercent(afterHF, inst.getOptValue());
				String str = "\\midrule " 
					+ inst.getName() + " & " 
					+ inst.getOptValue() + " & "
					+ afterVNS + " & "
					+ afterHF + " & "
					+ String.format(Locale.ENGLISH, "%.2f", gap)
					+ " \\\\";
				System.out.println(str);
			}
			catch(FileNotFoundException fnfe)
			{
				fnfe.printStackTrace();
			}
		}

		System.out.println("\n");

		System.out.println("TABELLA ISTANZE");
		for(Instance inst : moduloList)
		{
			String nnodes = inst.getName().replaceAll("[^0-9]", "");
			String str = "\\midrule " 
				+ inst.getName() + " & " 
				+ nnodes + " & "
				+ inst.getOptValue()
				+ " \\\\";
			System.out.println(str);
		}
		
		System.out.println("Number of files = " + listFiles.length);
		
		/*
		for(Map.Entry<String, Long> entry : optValueMap.entrySet())
		{
			System.out.println(entry.getKey() + " -> " + entry.getValue());
		}
		*/
	}

	private static double gapPercent(double value, double optValue)
	{
		return (100 * (value - optValue) / optValue);
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
					//System.out.println(strArray[0] + "-->" + strArray[1]);
				}
			}
		} 
		catch (IOException ioe)
		{
			ioe.printStackTrace();
		}
	}

	
}