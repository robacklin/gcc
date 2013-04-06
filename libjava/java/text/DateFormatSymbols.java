/* Copyright (C) 1998, 1999, 2000  Free Software Foundation

   This file is part of libgcj.

This software is copyrighted work licensed under the terms of the
Libgcj License.  Please consult the file "LIBGCJ_LICENSE" for
details.  */

package java.text;

import java.util.Locale;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

/**
 * @author Per Bothner <bothner@cygnus.com>
 * @date October 24, 1998.
 */

/* Written using "Java Class Libraries", 2nd edition, ISBN 0-201-31002-3.
 * Status:  Believed complete and correct.
 */

public class DateFormatSymbols extends Object
  implements java.io.Serializable, Cloneable
{
  String[] ampms;
  String[] eras;
  private String localPatternChars;
  String[] months;
  String[] shortMonths;
  String[] shortWeekdays;
  String[] weekdays;
  private String[][] zoneStrings;

  private static final long serialVersionUID = -5987973545549424702L;

  // The order of these prefixes must be the same as in DateFormat
  // FIXME: XXX: Note that this differs from the Classpath implemention
  // in that there is no "default" entry; that is due to differing
  // implementations where DateFormat.DEFAULT is MEDIUM here but 4 in
  // Classpath (the JCL says it should be MEDIUM).  That will need to be 
  // resolved in the merge.
  private static final String[] formatPrefixes = { "full", "long", "medium", "short" };

  private static final String[] ampmsDefault = {"AM", "PM" };
  private static final String[] erasDefault = {"BC", "AD" };
  // localPatternCharsDefault is used by SimpleDateFormat.
  private static final String localPatternCharsDefault
    = "GyMdkHmsSEDFwWahKz";
  private static final String[] monthsDefault = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December", ""
  };
  private static final String[] shortMonthsDefault = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", ""
  };
  private static final String[] shortWeekdaysDefault = {
    "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  private static final String[] weekdaysDefault = {
    "", "Sunday", "Monday", "Tuesday",
    "Wednesday", "Thursday", "Friday", "Saturday"
  };

  private static String[][] zoneStringsDefault = {
    { "GMT", "Greenwich Mean Time", "GMT",
      /**/   "Greenwich Mean Time", "GMT", "GMT" },
    { "PST", "Pacific Standard Time", "PST",
      /**/   "Pacific Daylight Time", "PDT", "San Francisco" },
    { "MST", "Mountain Standard Time", "MST",
      /**/   "Mountain Daylight Time", "MDT", "Denver" },
    { "PNT", "Mountain Standard Time", "MST",
      /**/   "Mountain Standard Time", "MST", "Phoenix" },
    { "CST", "Central Standard Time", "CST",
      /**/   "Central Daylight Time", "CDT", "Chicago" },
    { "EST", "Eastern Standard Time", "EST",
      /**/   "Eastern Daylight Time", "EDT", "Boston" },
    { "IET", "Eastern Standard Time", "EST",
      /**/   "Eastern Standard Time", "EST", "Indianapolis" },
    { "PRT", "Atlantic Standard Time", "AST",
      /**/   "Atlantic Daylight Time", "ADT", "Halifax" },
    { "CNT", "Newfoundland Standard Time", "NST",
      /**/   "Newfoundland Daylight Time", "NDT", "St. Johns" },
    { "ECT", "Central European Standard Time", "CET",
      /**/   "Central European Daylight Time", "CEST", "Paris" },
    { "CTT", "China Standard Time", "CST",
      /**/   "China Standard Time", "CST", "Shanghai" },
    { "JST", "Japan Standard Time", "JST",
      /**/   "Japan Standard Time", "JST", "Tokyo" },
    { "HST", "Hawaii Standard Time", "HST",
      /**/   "Hawaii Standard Time", "HST", "Honolulu" },
    { "AST", "Alaska Standard Time", "AKST",
      /**/   "Alaska Daylight Time", "AKDT", "Anchorage" }
  };

  // These are each arrays with a value for SHORT, MEDIUM, LONG, FULL,
  // and DEFAULT (constants defined in java.text.DateFormat).  While
  // not part of the official spec, we need a way to get at locale-specific
  // default formatting patterns.  They are declared package scope so
  // as to be easily accessible where needed (DateFormat, SimpleDateFormat).
  transient String[] dateFormats;
  transient String[] timeFormats;

  private String[] formatsForKey(ResourceBundle res, String key) 
  {
    String[] values = new String [formatPrefixes.length];
    for (int i = 0; i < formatPrefixes.length; i++)
      {
        values[i] = res.getString(formatPrefixes[i]+key);
      }
    return values;
  }

  private final Object safeGetResource (ResourceBundle res,
					String key, Object def)
  {
    if (res != null)
      {
	try
	  {
	    return res.getObject(key);
	  }
	catch (MissingResourceException x)
	  {
	  }
      }
    return def;
  }

  public DateFormatSymbols (Locale locale)
  {
    ResourceBundle res;
    try
      {
	res = ResourceBundle.getBundle("gnu.gcj.text.LocaleData", locale);
      }
    catch (MissingResourceException x)
      {
	res = null;
      }
    ampms = (String[]) safeGetResource (res, "ampm", ampmsDefault);
    eras = (String[]) safeGetResource (res, "eras", erasDefault);
    localPatternChars = (String) safeGetResource (res, "datePatternChars",
						  localPatternCharsDefault);
    months = (String[]) safeGetResource (res, "months", monthsDefault);
    shortMonths = (String[]) safeGetResource (res, "shortMonths",
					      shortMonthsDefault);
    shortWeekdays = (String[]) safeGetResource (res, "shortWeekdays",
						shortWeekdaysDefault);
    weekdays = (String[]) safeGetResource (res, "weekdays", weekdaysDefault);
    zoneStrings = (String[][]) safeGetResource (res, "zoneStrings",
						zoneStringsDefault);

    dateFormats = formatsForKey(res, "DateFormat");
    timeFormats = formatsForKey(res, "TimeFormat");
  }

  public DateFormatSymbols ()
  {
    this (Locale.getDefault());
  }

  // Copy constructor.
  private DateFormatSymbols (DateFormatSymbols old)
  {
    ampms = old.ampms;
    eras = old.eras;
    localPatternChars = old.localPatternChars;
    months = old.months;
    shortMonths = old.shortMonths;
    shortWeekdays = old.shortWeekdays;
    weekdays = old.weekdays;
    zoneStrings = old.zoneStrings;
    dateFormats = old.dateFormats;
    timeFormats = old.timeFormats;
  }

  public String[] getAmPmStrings()
  {
    return ampms;
  }

  public String[] getEras()
  {
    return eras;
  }


  public String getLocalPatternChars()
  {
    return localPatternChars;
  }

  public String[] getMonths ()
  {
    return months;
  }

  public String[] getShortMonths ()
  {
    return shortMonths;
  }

  public String[] getShortWeekdays ()
  {
    return shortWeekdays;
  }

  public String[] getWeekdays ()
  {
    return weekdays;
  }

  public String[] [] getZoneStrings ()
  {
    return zoneStrings;
  }

  public void setAmPmStrings (String[] value)
  {
    ampms = value;
  }

  public void setEras (String[] value)
  {
    eras = value;
  }

  public void setLocalPatternChars (String value)
  {
    localPatternChars = value;
  }

  public void setMonths (String[] value)
  {
    months = value;
  }

  public void setShortMonths (String[] value)
  {
    shortMonths = value;
  }

  public void setShortWeekdays (String[] value)
  {
    shortWeekdays = value;
  }

  public void setWeekdays (String[] value)
  {
    weekdays = value;
  }

  public void setZoneStrings (String[][] value)
  {
    zoneStrings = value;
  }

  /* Does a "deep" equality test - recurses into arrays. */
  private static boolean equals (Object x, Object y)
  {
    if (x == y)
      return true;
    if (x == null || y == null)
      return false;
    if (! (x instanceof Object[]) || ! (y instanceof Object[]))
      return x.equals(y);
    Object[] xa = (Object[]) x;
    Object[] ya = (Object[]) y;
    if (xa.length != ya.length)
      return false;
    for (int i = xa.length;  --i >= 0; )
      {
	if (! equals(xa[i], ya[i]))
	  return false;
      }
    return true;
  }

  private static int hashCode (Object x)
  {
    if (x == null)
      return 0;
    if (! (x instanceof Object[]))
      return x.hashCode();
    Object[] xa = (Object[]) x;
    int hash = 0;
    for (int i = 0;  i < xa.length;  i++)
      hash = 37 * hashCode(xa[i]);
    return hash;
  }

  public boolean equals (Object obj)
  {
    if (obj == null || ! (obj instanceof DateFormatSymbols))
      return false;
    DateFormatSymbols other = (DateFormatSymbols) obj;
    return (equals(ampms, other.ampms)
	    && equals(eras, other.eras)
	    && equals(localPatternChars, other.localPatternChars)
	    && equals(months, other.months)
	    && equals(shortMonths, other.shortMonths)
	    && equals(shortWeekdays, other.shortWeekdays)
	    && equals(weekdays, other.weekdays)
	    && equals(zoneStrings, other.zoneStrings));
  }

  public Object clone ()
  {
    return new DateFormatSymbols (this);
  }

  public int hashCode ()
  {
    return (hashCode(ampms)
	    ^ hashCode(eras)
	    ^ hashCode(localPatternChars)
	    ^ hashCode(months)
	    ^ hashCode(shortMonths)
	    ^ hashCode(shortWeekdays)
	    ^ hashCode(weekdays)
	    ^ hashCode(zoneStrings));
  }
}
