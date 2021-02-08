package kdd

import java.awt.{Dimension, Event, GridLayout}
import java.awt.event.{MouseWheelEvent, MouseWheelListener}
import java.io.{File, PrintWriter}

import breeze.numerics.pow
import javax.swing.{JFrame, JPanel, WindowConstants}
import org.apache.spark.SparkContext
import org.apache.spark.SparkConf
import org.apache.spark.graphx._
import org.apache.spark.graphx.util.GraphGenerators
import org.apache.log4j.Logger
import org.apache.log4j.Level
import org.apache.spark.rdd.RDD
import org.graphstream.graph.implementations.{AbstractEdge, SingleGraph, SingleNode}

import scala.concurrent.duration._
import scala.collection.mutable.ArrayBuffer
import scala.io.Source
import org.graphstream.graph.{Graph => GraphStream}
import org.graphstream.ui.swingViewer.{View, Viewer}

import scala.collection.mutable.Map
import scala.util.control.Breaks._
import scala.collection.immutable.Range

private object SocialNetworkAnalyze {


  def main(args: Array[String]) {
    Logger.getLogger("org").setLevel(Level.ERROR)
    Logger.getLogger("akka").setLevel(Level.ERROR)

    val conf = new SparkConf().setAppName("SocialNetWorkAnalyze").setMaster("local[*]").set("driver-memory", "5g")
    val sc = new SparkContext(conf)

    //Get args
    val filename = args(0)
    val nodenum = args(1).toLong
    val resultpath = "GlobalProperty.txt"
    val timestart = args(2).toInt
    val timeend = args(3).toInt
    val timegap = args(4).toInt
    val groupfile = "./Group/"
    val relationfile = "./Relation/"
    val degreefile = "./Degree/"
    val distfile = "./Dist/"

    val groupdir = new File(groupfile)
    if(!groupdir.exists()){
      groupdir.mkdirs()
    }
    val relationdir = new File(relationfile)
    if(!relationdir.exists()){
      relationdir.mkdirs()
    }
    val degreedir = new File(degreefile)
    if(!degreedir.exists()){
      degreedir.mkdirs()
    }
    val distdir = new File(distfile)
    if(!distdir.exists()){
      distdir.mkdirs()
    }


    var vertexArray = ArrayBuffer(
      (1L, 1L)
    )
    vertexArray.trimEnd(1)

    for(i <- 0L until nodenum){
      vertexArray+=new Tuple2(i, 1L)
    }

    val vertexRDD: RDD[(Long, Long)] = sc.parallelize(vertexArray)



    var source = Source.fromFile(filename)
    var lineIterator = source.getLines()
    var line = lineIterator.next().split(" ")


    var timeEdge:Map[String, Array[Double]] = Map()


    val globalres = new PrintWriter(new File(resultpath))



    for(i <- Range(timestart,timeend,timegap)){
      val start = System.currentTimeMillis()

      val timeLimit = (i+1)*24

      var flag = true


      while(flag){

        if(line(2).toInt <= timeLimit){

          if(timeEdge.contains(line(0) + ">" + line(1))) {
  
            val lastv = timeEdge(line(0) + ">" + line(1))(0)
            val lastt = timeEdge(line(0) + ">" + line(1))(1)
   
            val nowt = line(2).toDouble
   
            var lastvt = pow((1-lastv)/0.56, 1/0.06)
   
            var nowv = 1 - 0.28*pow(lastvt + nowt - lastt, 0.06)
   
            timeEdge(line(0) + ">" + line(1))(0) = nowv
            timeEdge(line(0) + ">" + line(1))(1) = nowt
          }
          else if(timeEdge.contains(line(1) + ">" + line(0))){
  
            val lastv = timeEdge(line(1) + ">" + line(0))(0)
            val lastt = timeEdge(line(1) + ">" + line(0))(1)
           
            val nowt = line(2).toDouble
 
            var lastvt = pow((1-lastv)/0.56, 1/0.06)
  
            var nowv = 1 - 0.28*pow(lastvt + nowt - lastt, 0.06)
      
            timeEdge(line(1) + ">" + line(0))(0) = nowv
            timeEdge(line(1) + ">" + line(0))(1) = nowt
          }
          else{
            
            timeEdge(line(0) + ">" + line(1)) = Array(1, line(2).toDouble)
          }
        }
        else{
          flag = false
        }
 
        if(lineIterator.hasNext){
          line = lineIterator.next.split(" ")
        }
        else{
          flag = false
        }
      }
     
      timeEdge.foreach{case (strs, value) =>{
        val lastv = value(0)
        val lastt = value(1)
        val nowt = timeLimit
        var lastvt = pow((1-lastv)/0.56, 1/0.06)
        var nowv = 1 - 0.56*pow(lastvt + nowt - lastt, 0.06)
        value(0) = nowv
        value(1) = nowt
      }}
   
      var edgeArray = ArrayBuffer(
        Edge(1L,1L,1.0)
      )
      edgeArray.trimEnd(1)

      //store relation
      val relationwriter = new PrintWriter(new File(relationfile+i.toString))
      timeEdge.foreach{case (strs, value) =>{
        val str = strs.split(">")
        if(value(0) > 0.2061){
          edgeArray += Edge(str(0).toLong, str(1).toLong, 1)
          edgeArray += Edge(str(1).toLong, str(0).toLong, 1)
          relationwriter.write(str(0) + " " + str(1) + " " + value(0).toString + "\n")
        }
      }}
      relationwriter.close()
   
      val edgeRDD: RDD[Edge[Double]] = sc.parallelize(edgeArray)
   
      val graph: Graph[Long, Double] = Graph(vertexRDD, edgeRDD)
      val graphend = System.currentTimeMillis()


      val cnode = graph.connectedComponents().vertices

      val cctemp = cnode.groupBy(x => x._2).filter{case (x,y) => y.size > 2}
   
      val ccnumber = cctemp.keys
 
      val cccount = ccnumber.count()

      val apspst = System.currentTimeMillis()
 
      val apsp = new ParellelBlockFW
      var result = time {
        apsp.compute(graph, 250)
      }
      val apspend = System.currentTimeMillis()

     
      var restats = result.distMatrix.blocks.flatMap{case(_,x) => x.toArray}.filter(x => x != 0 && x != Double.PositiveInfinity).collect()

      val distwriter = new PrintWriter(new File(distfile+i.toString))
      for(dist <- restats){
        distwriter.write(dist.toString + "\n")
      }
      distwriter.close()

   
      var resMatrix = result.toLocal()

   
      val censtart = System.currentTimeMillis()
      for(j <- ccnumber.collect()){
      
        var node = cnode.filter(x => x._2==j).keys
        val nodenum = node.count()
     
        var max = 0.0


        var nclist:Map[Int, Double] = Map()

       
        val nodes = node.collect()
        for(k <- nodes){
      
          var sum = 0.0
          for(l <- nodes){
            if(l != k){
              val dist = resMatrix.apply(k.toInt,l.toInt)
              if(max < dist){
              
                max = dist
              }
              sum += dist
            }
          }
        
          val nodecenter = (nodenum-1)/sum
          nclist(k.toInt) = nodecenter
        }
        node.unpersist()
      
        val groupwriter = new PrintWriter(new File(groupfile+i.toString + " " + j.toString))
        groupwriter.write(nodenum.toString + "\n")
        groupwriter.write(max.toString + "\n")
        for(l <- nclist){
          groupwriter.write(l._1.toString+ " " + l._2.toString + "\n")
        }
        groupwriter.close()
      }
      val cenEnd = System.currentTimeMillis()
 
      val degree = graph.inDegrees.values.collect()
      val degreewriter = new PrintWriter(new File(degreefile+i.toString))
      for(degree <- degree){
        degreewriter.write(degree.toString + "\n")
      }
      degreewriter.close()

      val zerodegree = nodenum - graph.inDegrees.count()
      val groupnum = cccount

      globalres.write(i.toString + " " + zerodegree.toString + " " + groupnum.toString + " " + restats.max.toString + "\n")
      //println("Days: " + i.toString +"    AloneNode: "+zerodegree.toString +"    Groups: "+groupnum.toString+"    AverageDegree: "+avedegree.toString + "    MeanDist: "+restats.mean.toString+"    Diameter: "+restats.max.toString)

      println("Days: " + i.toString + " graphTime: " + (graphend-start).toString + " propertyTime: " + (apspst-graphend).toString + " apspTime:  " + (apspend-apspst).toString + " cenTime: " + (cenEnd-censtart) )

      ccnumber.unpersist()
      cctemp.unpersist()
      cnode.unpersist()
      graph.unpersist()
      edgeRDD.unpersist()
      vertexRDD.unpersist()

      result = null
      resMatrix = null
      restats = null
    }
    globalres.close()
    source.close()
    //System.exit(0)
  }
  def time[R](block: => R): R = {
    val t0 = System.nanoTime()
    val result = block 
    val t1 = System.nanoTime()
    val duration = Duration(t1 - t0, NANOSECONDS)
	
    result
  }
}
