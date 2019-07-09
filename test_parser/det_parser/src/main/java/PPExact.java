import java.util.*;
import java.io.*;

public class PPExact
{
	private final String DIRECTORY_PATH = "src/main/resources/alldet";
	private final String PP_PATH = "src/main/resources/PerfProf/";
	
	List<Instance> instanceList = new ArrayList<>();

	public static void main(String[] args) throws IOException
	{ (new PPExact()).exec(args); }

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
 				double ticks = 2127600.0;

 				if(isEmpty(instance.getFile()))
 				{
 					isOpt = "no";
 					ticks = 2127600.0;
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
 							ticks = Double.parseDouble(sc.nextLine().split(" ")[5]);
 							break;
 						}
 					}
 				}

				if(fileName.split("-")[3].equals("1"))
				{
					instance.setAvgMean(ticks/3);
 					instance.setGeoMean(Math.pow(ticks, (1/3.0)));
 					instance.setIsOpt(isOpt);
 					//System.out.println("inserisco");
					instanceList.add(instance);
				}
				else
				{
					for(Instance instInside : instanceList)
					{
						if(instInside.compareTo(instance) == 0)
						{
							//System.out.println("Aggiorno");
							instInside.setAvgMean(instInside.getAvgMean() + ticks/3);
 							instInside.setGeoMean(instInside.getGeoMean() * Math.pow(ticks, (1/3.0)));
 							if(isOpt.equals("yes"))
 								instInside.setIsOpt(isOpt);
						}
					}
				}
				//if(i++ == 18)
				//	break;
			}
		}

		Map<Instance, Double> instanceMap = new TreeMap<>();
		for(Instance inst : instanceList)
			instanceMap.put(inst, inst.getGeoMean());

		printPP(instanceMap, 12, true, PP_PATH + "pp_exact_12.csv");

		printPP(instanceMap, 893, true, PP_PATH + "pp_exact_893.csv");

		printPP(instanceMap, 39848, true, PP_PATH + "pp_exact_39848.csv");

		printPP(instanceMap, 2648948, true, PP_PATH + "pp_exact_2648948.csv");

		printPP(instanceMap, 201709013, true, PP_PATH + "pp_exact_201709013.csv");

		printPP(instanceMap, 9837745292L, true, PP_PATH + "pp_exact_9837745292.csv");

		System.out.println("\n");
		
		System.out.println("Number of files = " + listFiles.length);
	}


	private static void printPP(Map<Instance, Double> instanceMap, long randomSeed, boolean writeToCSV, String fileName) throws IOException
	{
		String header = "3, Loop Method, Callback Method, UserCutCallback Method";
		PrintWriter pw = null;
		if(writeToCSV)
		{
			pw = new PrintWriter(new File(fileName));
			pw.println(header);
		}
		System.out.println(header);
		for(Map.Entry<Instance, Double> entry : instanceMap.entrySet())
		{
			Instance current = entry.getKey();
			if(current.getMethod().equals("sec_loop") && current.getRandomSeed() == randomSeed)
			{
				double callback = instanceMap.get(new Instance(current.getName(), "sec_callback", current.getRandomSeed()));
				double usercut = instanceMap.get(new Instance(current.getName(), "usr_callback", current.getRandomSeed()));

				String toPrint = current.getName() + ", " 
					+ String.format(Locale.ENGLISH, "%.6f", current.getGeoMean()) + ", " 
					+ String.format(Locale.ENGLISH, "%.6f", callback) + ", " 
					+ String.format(Locale.ENGLISH, "%.6f", usercut);
				System.out.println(toPrint);

				if(writeToCSV)
				{
					pw.println(toPrint);
					pw.flush();
				}

			}
		}
	}


	public static boolean isEmpty(File file) throws IOException
	{
		BufferedReader br = new BufferedReader(new FileReader(file));
		return (br.readLine() == null)? true : false;
	}
}