package edu.gatech.c4g.r4g.model;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Hashtable;

import com.vividsolutions.jts.geom.Coordinate;
import com.vividsolutions.jts.geom.Geometry;
import com.vividsolutions.jts.geom.MultiPolygon;
import com.vividsolutions.jts.geom.Point;

public class Island extends Graph {

	public Island(Collection<Block> blocks) {
		this.blocks = new Hashtable<Integer, Block>();
		addAllBlocks(blocks);
	}

	public Coordinate getCenter() {
		double x = 0;
		double y = 0;

		for (Block b : blocks.values()) {
			x += b.getPolygon().getCentroid().getX();
			y += b.getPolygon().getCentroid().getY();
		}

		x = x / blocks.size();
		y = y / blocks.size();

		return new Coordinate(x, y);
	}
	
	/**
	 * Returns a list containing the blocks that are located on the boundary of
	 * this island
	 * 
	 * @return the list of blocks located on the boundary
	 */
	public HashSet<Block> findBoundaryBlocks(){
		HashSet<Block> boundary = new HashSet<Block>();
		
		for (Block b : blocks.values()){
			Geometry bBoundary = b.getPolygon().getBoundary();
			
			int count = bBoundary.getNumPoints();
			
			for (Block n : b.neighbors){
				bBoundary = bBoundary.difference(n.getPolygon().getBoundary());
			}
			
			count = bBoundary.getNumPoints();
			if (count > 0){
				boundary.add(b);
			}
		}
		
		return boundary;
	}
	
	public Block getRepresentative(){
		ArrayList<Block> allBlocks = new ArrayList<Block>(blocks.values());
		return allBlocks.get(0);
	}
}
