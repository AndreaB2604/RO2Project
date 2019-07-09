import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;

import java.util.*;
import java.io.*;

public class ParserHeur
{
	public static void main(String[] args)
	{
		(new ParserHeur()).exec(args);
	}
	
	void exec(String[] args)
	{
		File folder = new File("src/main/resources/runheur");

		Map<String, Long> optValueMap = new TreeMap<>(); 
		parseHTML(optValueMap);

		File[] listFiles = folder.listFiles();
		Arrays.sort(listFiles);
		List<Instance> heurInstanceList = new ArrayList<>(listFiles.length);
		List<Instance> matheurInstanceList = new ArrayList<>(listFiles.length);
		for(File file : listFiles) 
		{
			if(!file.isDirectory())
			{
				Instance instance = new Instance();
				instance.setFile(file);
				
				String fileName = file.getName();
				
				String name = fileName.split("\\.")[0];
				instance.setName(name);
				instance.setOptValue(optValueMap.get(name));

				String method = fileName.split("-")[1];
				instance.setMethod(method);
				if(method.equals("heur_hf") || method.equals("heur_lb"))
					matheurInstanceList.add(instance);
				else if(method.equals("heur_vns") || method.equals("heur_tabu"))
					heurInstanceList.add(instance);
			}
		}

		System.out.println("TABELLA MATEURISTICI");
		for(Instance inst : matheurInstanceList)
		{
			Scanner sc = null;
			try
			{
				sc = new Scanner(inst.getFile());
				
				int i = -1;
				long lastValue = -1;
				while(sc.hasNextLine())
				{
					String line = sc.nextLine();
					//System.out.println(line);
					if(line.startsWith("solution value at "))
					{
						lastValue = (long) Double.parseDouble(line.split(" ")[5]);
						inst.setValues(inst.getValues() + " & " + lastValue);
						if(i == 3)
							break;
						i++;
					}
				}
				if(i < 3)
				{
					String str = (lastValue >= 0)? Long.toString(lastValue) : " - ";
					for(int j = i; j < 3; j++)
						inst.setValues(inst.getValues() + " & " + str);
				}
			}
			catch(FileNotFoundException fnfe)
			{
				fnfe.printStackTrace();
			}
			if(inst.getMethod().equals("heur_hf"))
			{
				double gap = gapPercent(Double.parseDouble(inst.getValues().split(" ")[8]), inst.getOptValue());
				String toPrint = "\\midrule " 
					+ inst.getName() + " & " 
					+ inst.getOptValue()  
					+ inst.getValues() + " & " 
					+ String.format(Locale.ENGLISH, "%.2f", gap);
				System.out.print(toPrint);
			}
			else
			{
				//System.out.println("getValues = " + inst.getValues());
				try
				{
					double gap = gapPercent(Double.parseDouble(inst.getValues().split(" ")[8]), inst.getOptValue());
					String toPrint = inst.getValues() + " & " 
						+ String.format(Locale.ENGLISH, "%.2f", gap) + " \\\\";
					System.out.println(toPrint);
				}
				catch(NumberFormatException nfe)
				{
					String toPrint = inst.getValues() + " & 100.00" + " \\\\";
					System.out.println(toPrint);
				}
			}
		}

		System.out.println("\n");

		System.out.println("TABELLA EURISTICI");
		for(Instance inst : heurInstanceList)
		{
			Scanner sc = null;
			try
			{
				sc = new Scanner(inst.getFile());
				
				int i = -1;
				long lastValue = -1;
				while(sc.hasNextLine())
				{
					String line = sc.nextLine();
					//System.out.println(line);
					if(line.startsWith("solution value at "))
					{
						lastValue = (long) Double.parseDouble(line.split(" ")[5]);
						inst.setValues(inst.getValues() + " & " + lastValue);
						if(i == 3)
							break;
						i++;
					}
				}
				if(i < 3)
				{
					String str = (lastValue >= 0)? Long.toString(lastValue) : " - ";
					for(int j = i; j < 3; j++)
						inst.setValues(inst.getValues() + " & " + str);
				}
			}
			catch(FileNotFoundException fnfe)
			{
				fnfe.printStackTrace();
			}
			if(inst.getMethod().equals("heur_tabu"))
			{
				//System.out.println("getValues = " + inst.getValues().split(" ")[8]);
				double gap = gapPercent(Double.parseDouble(inst.getValues().split(" ")[8]), inst.getOptValue());
				String toPrint = "\\midrule " 
					+ inst.getName() + " & " 
					+ inst.getOptValue()  
					+ inst.getValues() + " & " 
					+ String.format(Locale.ENGLISH, "%.2f", gap);
				System.out.print(toPrint);
			}
			else
			{
				double gap = gapPercent(Double.parseDouble(inst.getValues().split(" ")[8]), inst.getOptValue());
				String toPrint = inst.getValues() + " & " 
					+ String.format(Locale.ENGLISH, "%.2f", gap) + " \\\\";
				System.out.println(toPrint);
			}
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

	private class Instance
	{
		private int t1, t2, t3;
		private long optValue;
		private String name, method, values;
		private File file;

		public Instance()
		{
			name = "";
			method = "";
			file = null;
			values = "";
			optValue = -1;
		}

		public Instance(String name, File file, String method, long optValue, String values)
		{
			this.name = name;
			this.method = method;
			this.file = file;
			this.optValue = optValue;
			this.values = values;
		}

		public void setName(String name)
		{ this.name = name; }

		public void setMethod(String method)
		{ this.method = method; }

		public void setFile(File file)
		{ this.file = file; }

		public void setOptValue(long optValue)
		{ this.optValue = optValue; }

		public void setValues(String values)
		{ this.values = values; }

		public String getName()
		{ return name; }

		public String getMethod()
		{ return method; }

		public File getFile()
		{ return file; }

		public long getOptValue()
		{ return optValue; }

		public String getValues()
		{ return values; }

		@Override
		public String toString()
		{
			return t1 + " & " + t2 + " & " + t3;
		}
	}
}