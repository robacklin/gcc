/* Copyright (C) 1999, 2000  Free Software Foundation

   This file is part of libjava.

This software is copyrighted work licensed under the terms of the
Libjava License.  Please consult the file "LIBJAVA_LICENSE" for
details.  */

package java.awt.geom;

/**
 * @author Per Bothner <bothner@cygnus.com>
 * @date February, 1999.
 */

/* Written using online API docs for JDK 1.2 beta from http://www.javasoft.com.
 * Status:  Believed complete and correct.
 */

public abstract class Dimension2D implements Cloneable
{
  public abstract double getWidth();
  public abstract double getHeight();

  public abstract void setSize (double width, double height);

  public void setSize (Dimension2D dim)
  {
    setSize(dim.getWidth(), dim.getHeight());
  }

  public Object clone ()
  {
    try
    {
      return super.clone ();
    } 
    catch (CloneNotSupportedException _) {return null;}
  }

  protected Dimension2D ()
  {
  }
}
