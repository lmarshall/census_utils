package edu.gatech.c4g.r4g.redistricting;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;

import org.geotools.data.FeatureSource;
import org.opengis.feature.simple.SimpleFeature;
import org.opengis.feature.simple.SimpleFeatureType;

import edu.gatech.c4g.r4g.model.Block;
import edu.gatech.c4g.r4g.model.BlockGraph;
import edu.gatech.c4g.r4g.model.District;
import edu.gatech.c4g.r4g.model.Island;
import edu.gatech.c4g.r4g.util.Loader;

public abstract class RedistrictingAlgorithm {

	double idealPopulation;
	double minPopulation;
	double maxPopulation;

	BlockGraph bg;
	ArrayList<Island> islands;
	Loader loader;

	public RedistrictingAlgorithm(Loader loader,
			FeatureSource<SimpleFeatureType, SimpleFeature> source,
			String galFile) {
		this.loader = loader;

		System.out.println("Loading files");
		bg = loader.load(source, galFile);

		// islands = new ArrayList<Island>();
		System.out.println("Finding islands");
		islands = bg.toIslands();
		System.out.println("Found " + islands.size() + " islands");

		// TEST
		// for (Island i : islands){
		// System.out.println(i.getPopulation());
		// }
	}

	/**
	 * TODO THIS DOES NOT WORK!! (WHY?)
	 * 
	 * @param ndis
	 *            number of districts to create
	 */
	public void redistrict(int ndis, double maxDeviation) {
		// calculate ideal population
		idealPopulation = bg.getPopulation() / ndis;
		minPopulation = idealPopulation - idealPopulation * maxDeviation;
		maxPopulation = idealPopulation + idealPopulation * maxDeviation;

		// STAGE 1
		// redistrict mainland
		ArrayList<Block> mainlandBlocks = new ArrayList<Block>();
		mainlandBlocks.addAll(islands.get(0).getAllBlocks());
		// sort the blocks by density
		Collections.sort(mainlandBlocks, new BlockDensityComparator());

		for (int currentDistNo = 1; currentDistNo <= ndis; currentDistNo++) {
			System.out.println("Building district " + currentDistNo);// TODO
			// transform
			// to
			// LOG
			// (log4j?)

			Block firstBlock = findFirstBlock(mainlandBlocks);

			District dist = new District(currentDistNo);
			// add the most populated block
			HashSet<Block> expandFrom = new HashSet<Block>();
			dist.addBlock(firstBlock);
			expandFrom.add(firstBlock);

			// the condition here must be fixed
			while (!expandFrom.isEmpty()
					&& dist.getPopulation() <= idealPopulation) {

				ArrayList<Block> candidates = new ArrayList<Block>();

				for (Block b : expandFrom) {
					for (Block n : b.neighbors) {
						if (n.getDistNo() == Block.UNASSIGNED) {
							candidates.add(n);
						}
					}
				}

				HashSet<Block> blocksToAdd = chooseNeighbors(dist
						.getPopulation(), candidates);
				dist.addAllBlocks(blocksToAdd);
				expandFrom = blocksToAdd;
			}

			bg.addDistrict(dist);
		}

		// TEST
		double totPop = bg.getPopulation();

		for (District d : bg.getDistList()) {
			System.out.println("District " + d.getDistrictNo()
					+ ": population " + d.getPopulation() + "("
					+ (d.getPopulation() / totPop) * 100 + "%) ("
					+ d.getAllBlocks().size() + " blocks)");
		}

		System.out.println("Unassigned blocks: " + mainlandBlocks.size());

		// stage2

		// stage3
	}

	/**
	 * Returns the first unassigned block. The block list should be ordered by
	 * density using the {@link BlockDensityComparator}.
	 * 
	 * @return
	 */
	private Block findFirstBlock(ArrayList<Block> list) {
		for (Block b : list) {
			if (b.getDistNo() == Block.UNASSIGNED) {
				return b;
			}
		}

		return null;
	}

	private HashSet<Block> chooseNeighbors(int basePop, ArrayList<Block> blocks) {
		HashSet<Block> blocksToTake = new HashSet<Block>();

		int[] population = new int[blocks.size()];

		// generate random instance, items 1..N
		for (int n = 0; n < blocks.size(); n++) {
			population[n] = blocks.get(n).getPopulation();
		}

		// opt[n] = population obtained by taking blocks 1..n
		// sol[n] = does opt solution to pack items 1..n include item n?
		int[] opt = new int[blocks.size()];
		boolean[] sol = new boolean[blocks.size()];

		for (int n = 0; n < blocks.size(); n++) {
			// don't take block n
			int option1;
			// take item n
			int option2 = Integer.MIN_VALUE;

			if (n > 0) {
				option1 = opt[n - 1];
				if (population[n] + population[n - 1] <= maxPopulation)
					option2 = population[n] + opt[n - 1];
			} else {
				option1 = basePop;
				if (population[n] + basePop <= maxPopulation)
					option2 = population[n] + basePop;
			}

			// select better of two options
			opt[n] = Math.max(option1, option2);
			sol[n] = (option2 >= option1);
		}

		// determine which items to take
		for (int n = blocks.size() - 1; n >= 0; n--) {
			if (sol[n]) {
				blocksToTake.add(blocks.get(n));
			}
		}

		return blocksToTake;
	}

	protected class BlockDensityComparator implements Comparator<Block> {

		public int compare(Block o1, Block o2) {
			if (o1.getDensity() > o2.getDensity()) {
				return -1;
			} else if (o1.getDensity() < o2.getDensity()) {
				return 1;
			}
			return 0;
		}

	}
}
