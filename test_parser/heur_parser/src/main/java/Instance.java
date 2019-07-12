import java.io.*;
import java.util.*;

public class Instance
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