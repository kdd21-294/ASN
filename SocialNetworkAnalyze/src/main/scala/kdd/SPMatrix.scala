package kdd

import java.io.Serializable

import org.apache.spark.mllib.linalg.Matrix
import org.apache.spark.mllib.linalg.distributed.BlockMatrix
import org.apache.spark.internal.Logging
import org.apache.spark.storage.StorageLevel


class SPMatrix(var size: Long, var distMatrix: BlockMatrix) extends Serializable with Logging{

  validateResult(distMatrix)

  private def validateResult(result: BlockMatrix): Unit = {
    require(result.numRows == result.numCols, "Result Matrix is not square.")
    require(size == result.numRows, "The size of the ResultMatrix is wrong.")

  }

  def lookupDist(srcId: Long, dstId: Long): Int = {
    val sizePerBlock = distMatrix.rowsPerBlock
    val rowBlockId = (srcId/sizePerBlock).toInt
    val colBlockId = (dstId/sizePerBlock).toInt
    val block = distMatrix.blocks.filter{case ((i, j), _) => ( i == rowBlockId) & (j == colBlockId)}.first._2
    block.toArray((dstId % sizePerBlock).toInt * block.numRows + (srcId % sizePerBlock).toInt).toInt
  }

  def toLocal(): Matrix = {
    distMatrix.toLocalMatrix()
  }
}
