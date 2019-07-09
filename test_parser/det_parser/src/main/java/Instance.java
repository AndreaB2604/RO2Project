import java.util.*;
import java.io.*;

public class Instance implements Comparable<Instance>
{
	private double avgMean, geoMean;
	private long randomSeed;
	private String name, method, isOpt;
	private File file;

	public Instance()
	{
		name = "";
		method = "";
		file = null;
		isOpt = "";
		randomSeed = -1;
		avgMean = 0;
		geoMean = 1;
	}

	public Instance(String name, String method, long randomSeed)
	{
		this.name = name;
		this.method = method;
		file = null;
		isOpt = "";
		this.randomSeed = randomSeed;
		avgMean = 0;
		geoMean = 1;
	}

	public void setName(String name)
	{ this.name = name; }

	public void setMethod(String method)
	{ this.method = method; }

	public void setFile(File file)
	{ this.file = file; }

	public void setIsOpt(String isOpt)
	{ this.isOpt = isOpt; }

	public void setRandomSeed(long randomSeed)
	{ this.randomSeed = randomSeed; }

	public void setAvgMean(double avgMean)
	{ this.avgMean = avgMean; }

	public void setGeoMean(double geoMean)
	{ this.geoMean = geoMean; }

	public String getName()
	{ return name; }

	public String getMethod()
	{ return method; }

	public File getFile()
	{ return file; }

	public String getIsOpt()
	{ return isOpt; }

	public long getRandomSeed()
	{ return randomSeed; }

	public double getAvgMean()
	{ return avgMean; }

	public double getGeoMean()
	{ return geoMean; }

	@Override
	public int compareTo(Instance inst)
	{
		int c = this.getName().compareTo(inst.getName());
		if(c != 0)
			return c;
		c = this.getMethod().compareTo(inst.getMethod());
		if(c != 0)
			return c;
		c = (new Long(this.getRandomSeed())).compareTo(new Long(inst.getRandomSeed()));
		return c;
	}
}