package kdd

import org.apache.spark.mllib.linalg.distributed.{BlockMatrix, CoordinateMatrix, MatrixEntry}
import org.apache.spark.mllib.linalg.{Matrix, Matrices, DenseMatrix, SparseMatrix}
import breeze.linalg.{DenseMatrix => BDM, DenseVector, min, Matrix =>BM}
import org.apache.spark.Partitioner
import org.apache.spark.rdd._
import org.apache.spark.internal.Logging
import java.io.Serializable


class MatrixPartitioner(val rows: Int, val cols: Int, val rowsPerPart: Int,
                       val colsPerPart: Int) extends Partitioner {
  require(rows > 0)
  require(cols > 0)
  require(rowsPerPart > 0)
  require(colsPerPart > 0)
  private val rowPartitions = math.ceil(rows * 1.0 / rowsPerPart).toInt
  private val colPartitions = math.ceil(cols * 1.0 / colsPerPart).toInt

  override val numPartitions: Int = rowPartitions * colPartitions

 
  override def getPartition(key: Any): Int = {
    key match {
      case (i: Int, j: Int) =>
        getPartitionId(i, j)
      case (i: Int, j: Int, _: Int) =>
        getPartitionId(i, j)
      case _ =>
        throw new IllegalArgumentException("[Error] Unrecognized key")
    }
  }

  private def getPartitionId(i: Int, j: Int): Int = {
    require(0 <= i && i < rows, "[Error] Row out of range.")
    require(0 <= j && j < cols, "[Error] Column out of range.")
    i / rowsPerPart + j / colsPerPart * rowPartitions
  }

  override def equals(obj: Any): Boolean = {
    obj match {
      case r: MatrixPartitioner =>
        (this.rows == r.rows) && (this.cols == r.cols) &&
          (this.rowsPerPart == r.rowsPerPart) && (this.colsPerPart == r.colsPerPart)
      case _ =>
        false
    }
  }


  override def hashCode: Int = {

    val chars = rows.toString + "," + cols.toString + "," + rowsPerPart.toString + "," + colsPerPart.toString
    chars.hashCode()
  }
}


object MatrixPartitioner {


  def apply(rows: Int, cols: Int, rowsPerPart: Int, colsPerPart: Int): MatrixPartitioner = {
    new MatrixPartitioner(rows, cols, rowsPerPart, colsPerPart)
  }


  def apply(rows: Int, cols: Int, suggestedNumPartitions: Int): MatrixPartitioner = {
    require(suggestedNumPartitions > 0)
    val scale = 1.0 / math.sqrt(suggestedNumPartitions)
    val rowsPerPart = math.round(math.max(scale * rows, 1.0)).toInt
    val colsPerPart = math.round(math.max(scale * cols, 1.0)).toInt
    new MatrixPartitioner(rows, cols, rowsPerPart, colsPerPart)
  }
}
