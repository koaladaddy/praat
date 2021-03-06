/* praat_FFNet_init.cpp
 *
 * Copyright (C) 1994-2011, 2016 David Weenink
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this work. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 djmw 20020408 GPL
 djmw 20020408 added FFNet_help
 djmw 20030701 Removed non-GPL minimizations
 djmw 20040526 Removed bug in FFNet_drawCostHistory interface.
 djmw 20041123 Latest modification
 djmw 20061218 Changed to Melder_information<x> format.
 djmw 20080902 Melder_error<1...>
 djmw 20071011 REQUIRE requires U"".
 djmw 20071024 Use MelderString_append in FFNet_createNameFromTopology
 djmw 20100511 FFNet query outputs
*/

#include <math.h>
#include "Discriminant.h"
#include "PCA.h"
#include "Minimizers.h"
#include "FFNet_Eigen.h"
#include "FFNet_Matrix.h"
#include "FFNet_PatternList.h"
#include "FFNet_ActivationList_Categories.h"
#include "FFNet_PatternList_ActivationList.h"
#include "FFNet_PatternList_Categories.h"
#include "RBM_extensions.h"

#include "praat_FFNet.h"

/* Routines to be removed sometime in the future:
20040422, 2.4.04: FFNet_drawWeightsToLayer  use FFNet_drawWeights
20040422, 2.4.04: FFNet_weightsToMatrix use FFNet_extractWeights
*/

#undef iam
#define iam iam_LOOP

static char32 const *QUERY_BUTTON   = U"Query -";
static char32 const *DRAW_BUTTON     = U"Draw -";
static char32 const *MODIFY_BUTTON  = U"Modify -";
static char32 const *EXTRACT_BUTTON = U"Extract -";

/**************** New FFNet ***************************/

FORM (NEW1_FFNet_create, U"Create FFNet", U"Create FFNet...") {
	WORDVAR (name, U"Name", U"4-3")
	NATURALVAR (numberOfInputs, U"Number of inputs", U"4")
	NATURALVAR (numberOfOutputs, U"Number of outputs", U"3")
	INTEGERVAR (numberOfUnits1, U"Number of units in hidden layer 1", U"0")
	INTEGERVAR (numberOfUnits2, U"Number of units in hidden layer 2", U"0")
	OK
DO
	CREATE_ONE
		autoFFNet result = FFNet_create (numberOfInputs, numberOfUnits1, numberOfUnits2, numberOfOutputs, false);
	CREATE_ONE_END (name);
}

FORM (NEWMANY_FFNet_createIrisExample, U"Create iris example", U"Create iris example...") {
	LABEL (U"", U"For the feedforward neural net we need to know the:")
	INTEGERVAR (numberOfUnits1, U"Number of units in hidden layer 1", U"0")
	INTEGERVAR (numberOfUnits2, U"Number of units in hidden layer 2", U"0")
	OK
DO
	autoCollection result = FFNet_createIrisExample (numberOfUnits1, numberOfUnits2);
	praat_new (result.move());
END }

FORM (NEW1_FFNet_create_linearOutputs, U"Create FFNet", U"Create FFNet (linear outputs)...") {
	WORDVAR (name, U"Name", U"4-3")
	NATURALVAR (numberOfInputs, U"Number of inputs", U"4")
	NATURALVAR (numberOfOutputs, U"Number of outputs", U"3")
	INTEGERVAR (numberOfUnits1, U"Number of units in hidden layer 1", U"0")
	INTEGERVAR (numberOfUnits2, U"Number of units in hidden layer 2", U"0")
	OK
DO
	CREATE_ONE
		autoFFNet result = FFNet_create (numberOfInputs, numberOfUnits1, numberOfUnits2, numberOfOutputs, true);
	CREATE_ONE_END (name);
}

/**************** New PatternList ***************************/

FORM (NEW1_PatternList_create, U"Create PatternList", nullptr) {
	WORDVAR (name, U"Name", U"1x1")
	NATURALVAR (patternDimension, U"Dimension of a pattern", U"1")
	NATURALVAR (numberOfPatterns, U"Number of patterns", U"1")
	OK
DO
	CREATE_ONE
		autoPatternList result = PatternList_create (numberOfPatterns, patternDimension);
	CREATE_ONE_END (name);
}
	
/**************** New Categories ***************************/
	
FORM (NEW1_Categories_create, U"Create Categories", nullptr) {
	WORDVAR (name, U"Name", U"empty")
	OK
DO
	CREATE_ONE
		autoCategories result = Categories_create ();
	CREATE_ONE_END (name);
}

DIRECT (HELP_FFNet_help)  {
	HELP (U"Feedforward neural networks")
}
		

DIRECT (GRAPHICS_FFNet_drawTopology) {
	GRAPHICS_EACH (FFNet)
		FFNet_drawTopology (me, GRAPHICS);
	GRAPHICS_EACH_END
}

FORM (GRAPHICS_FFNet_drawWeights, U"FFNet: Draw weights", U"FFNet: Draw weights...") {
	NATURALVAR (layer, U"Layer number", U"1")
	BOOLEANVAR (garnish, U"Garnish", true)
	OK
DO
	GRAPHICS_EACH (FFNet)
		FFNet_drawWeights (me, GRAPHICS, layer, garnish);
	GRAPHICS_EACH_END
}
	
FORM (GRAPHICS_FFNet_drawWeightsToLayer, U"FFNet: Draw weights to layer", nullptr) {
	LABEL (U"", U"Warning: Disapproved. Use \"Draw weights..\" instead.")
	NATURALVAR (layer, U"Layer number", U"1")
	RADIOVAR (scale, U"Scale", 1)
		RADIOBUTTON (U"by maximum of all weights to layer")
		RADIOBUTTON (U"by maximum weight from 'from-unit'")
		RADIOBUTTON (U"by maximum weight to 'to-unit'")
	BOOLEANVAR (garnish, U"Garnish", true)
	OK
DO
	GRAPHICS_EACH (FFNet)
		FFNet_drawWeightsToLayer (me, GRAPHICS, layer, scale, garnish);
	GRAPHICS_EACH_END
}

FORM (GRAPHICS_FFNet_drawCostHistory, U"FFNet: Draw cost history", U"FFNet: Draw cost history...") {
	INTEGERVAR (fromIteration, U"left Iteration_range", U"0")
	INTEGERVAR (toIteration, U"right Iteration_range", U"0")
	REALVAR (fromCost, U"left Cost_range", U"0.0")
	REALVAR (toCost, U"right Cost_range", U"0.0")
	BOOLEANVAR (garnish, U"Garnish", true)
	OK
DO
	GRAPHICS_EACH (FFNet)
		FFNet_drawCostHistory (me, GRAPHICS, fromIteration, toIteration, fromCost, toCost, garnish);
	GRAPHICS_EACH_END
}

DIRECT (INTEGER_FFNet_getNumberOfLayers) {
	INTEGER_ONE (FFNet)
		long result = my nLayers;
	INTEGER_ONE_END (U" layer", (my nLayers > 1 ? U"s" : U""))
}

DIRECT (INTEGER_FFNet_getNumberOfOutputs) {
	INTEGER_ONE (FFNet)
		long result = my nUnitsInLayer[my nLayers];
	INTEGER_ONE_END (U" units")
}


FORM (INTEGER_FFNet_getNumberOfHiddenUnits, U"FFNet: Get number of hidden units", U"FFNet: Get number of hidden units...") {
	NATURALVAR (layer, U"Hidden layer number", U"1")
	OK
DO
	INTEGER_ONE (FFNet)
		long result = layer > 0 && layer <= my nLayers - 1 ? my nUnitsInLayer[layer] : 0;
	INTEGER_ONE_END (U" units")
}

DIRECT (INTEGER_FFNet_getNumberOfInputs) {
	INTEGER_ONE (FFNet)
		long result = my nUnitsInLayer[0];
	INTEGER_ONE_END (U" units")
}

FORM (INTEGER_FFNet_getNumberOfHiddenWeights, U"FFNet: Get number of hidden weights", U"FFNet: Get number of hidden weights...") {
	NATURALVAR (layer, U"Hidden layer number", U"1")
	OK
DO
	INTEGER_ONE (FFNet)
		long result = (layer > 0 && layer <= my nLayers - 1) ? (my nUnitsInLayer[layer] * (my nUnitsInLayer[layer - 1] + 1)) : 0;
	INTEGER_ONE_END (U" weights (including biases)")
}
	
DIRECT (INTEGER_FFNet_getNumberOfOutputWeights) {
	INTEGER_ONE (FFNet)
		long result = my nUnitsInLayer[my nLayers] * (my nUnitsInLayer[my nLayers - 1] + 1);
	INTEGER_ONE_END (U" weights");
}

FORM (INFO_FFNet_getCategoryOfOutputUnit, U"FFNet: Get category of output unit", nullptr) {
	NATURALVAR (outputUnit, U"Output unit", U"1")
	OK
DO
	STRING_ONE (FFNet)
		const char32 *result = FFNet_getCategoryOfOutputUnit (me, outputUnit);
	STRING_ONE_END
}

FORM (INTEGER_FFNet_getOutputUnitOfCategory, U"FFNet: Get output unit of category", nullptr) {
	SENTENCEVAR (category, U"Category", U"u")
	OK
DO
	INTEGER_ONE (FFNet)
		long result = FFNet_getOutputUnitOfCategory (me, category);
	INTEGER_ONE_END (U" (output unit)")
}


FORM (REAL_FFNet_getBias, U"FFNet: Get bias", nullptr) {
	NATURALVAR (layer, U"Layer", U"1")
	NATURALVAR (unit, U"Unit", U"1")
	OK
DO
	NUMBER_ONE (FFNet)
		double result = FFNet_getBias (me, layer, unit);
	NUMBER_ONE_END (U" (bias)")
}


FORM (REAL_FFNet_getWeight, U"FFNet: Get weight", nullptr) {
	NATURALVAR (layer, U"Layer", U"1")
	NATURALVAR (unitTo, U"Unit", U"1")
	NATURALVAR (unitFrom, U"Unit from", U"1")
	OK
DO
	NUMBER_ONE (FFNet)
		double result = FFNet_getWeight (me, layer, unitTo, unitFrom);
	NUMBER_ONE_END (U"(weight between unit ", unitTo, U" in layer ", layer, U", and unit ", unitFrom, U"in layer ", layer - 1, U")")
}

DIRECT (REAL_FFNet_getMinimum) {
	NUMBER_ONE (FFNet)
		double result = FFNet_getMinimum (me);
	NUMBER_ONE_END (U" (minimum)");
}

FORM (MODIFY_FFNet_setBias, U"FFNet: Set bias", nullptr) {
	NATURALVAR (layer, U"Layer", U"1")
	NATURALVAR (unit, U"Unit", U"1")
	REALVAR (value, U"Value", U"0.0")
	OK
DO
	MODIFY_EACH (FFNet)
		FFNet_setBias (me, layer, unit, value);
	MODIFY_EACH_END 
}

FORM (MODIFY_FFNet_setWeight, U"FFNet: Set weight", nullptr) {
	NATURALVAR (layer, U"Layer", U"1")
	NATURALVAR (unitTo, U"Unit", U"1")
	NATURALVAR (unitFrom, U"Unit (from)", U"1")
	REALVAR (value, U"Value", U"0.0")
	OK
DO
	MODIFY_EACH (FFNet)
		FFNet_setWeight (me, layer, unitTo, unitFrom, value);
	MODIFY_EACH_END
}

FORM (MODIFY_FFNet_reset, U"FFNet: Reset", U"FFNet: Reset...") {
	LABEL (U"", U"Warning: this command destroys all previous learning.")
	LABEL (U"", U"New weights will be randomly chosen from the interval [-range, +range].")
	POSITIVEVAR (range, U"Range", U"0.1")
	OK
DO
	MODIFY_EACH (FFNet)
		FFNet_reset (me, range);
	MODIFY_EACH_END
}
	
FORM (MODIFY_FFNet_selectBiasesInLayer, U"FFNet: Select biases", U"FFNet: Select biases...") {
	LABEL (U"", U"WARNING: This command induces very specific behaviour ")
	LABEL (U"", U"during a following learning phase.")
	NATURALVAR (layer, U"Layer number", U"1")
	OK
DO
	MODIFY_EACH (FFNet)
		FFNet_selectBiasesInLayer (me, layer);
	MODIFY_EACH_END
}
	
DIRECT (MODIFY_FFNet_selectAllWeights) {
	MODIFY_EACH (FFNet)
		FFNet_selectAllWeights (me);
	MODIFY_EACH_END
}

FORM (NEW_FFNet_extractWeights, U"FFNet: Extract weights", U"FFNet: Extract weights...") {
	NATURALVAR (layer, U"Layer number", U"1")
	OK
DO
	CONVERT_EACH (FFNet)
		autoTableOfReal result = FFNet_extractWeights (me, layer);
	CONVERT_EACH_END (my name)
}

FORM (NEW_FFNet_weightsToMatrix, U"FFNet: Weights to Matrix ", nullptr) {
	LABEL (U"", U"Warning: Use \"Extract weights..\" instead.")
	NATURALVAR (layer, U"Layer number", U"1")
	OK
DO
	CONVERT_EACH (FFNet)
		autoMatrix result = FFNet_weightsToMatrix (me, layer, false);
	CONVERT_EACH_END (my name)
}

DIRECT (HINT_hint_FFNet_and_PatternList_classify) {
	Melder_information (U"You can use the FFNet as a classifier by selecting a\n"
	"FFNet and a PatternList together and choosing \"To Categories...\".");
END }
	
DIRECT (HINT_hint_FFNet_and_PatternList_and_Categories_learn) {
	Melder_information (U"You can teach a FFNet to classify by selecting a\n"
	"FFNet, a PatternList and a Categories together and choosing \"Learn...\".");
END }

/******************* FFNet && ActivationList ******************************************/

FORM (NEW1_FFNet_ActivationList_to_Categories, U"FFNet & ActivationList: To Categories", 0) {
	RADIOVAR (categorizationgMethod, U"Categorization method", 1)
		RADIOBUTTON (U"Winner-takes-all")
		RADIOBUTTON (U"Stochastic")
	OK
DO
	CONVERT_TWO (FFNet, ActivationList)
		autoCategories result = FFNet_ActivationList_to_Categories (me, you, categorizationgMethod);
	CONVERT_TWO_END (my name, U"_", you -> name)
}

/******************* FFNet && Eigen ******************************************/

FORM (GRAPHICS_FFNet_Eigen_drawIntersection, U"FFnet & Eigen: Draw intersection", 0) {
	NATURALVAR (pcx, U"X-component", U"1")
	NATURALVAR (pcy, U"Y-component", U"2")
	REALVAR (xmin, U"Xmin", U"0.0")
	REALVAR (xmax, U"Xmax", U"0.0")
	REALVAR (ymin, U"Ymin", U"0.0")
	REALVAR (ymax, U"Ymax", U"0.0")
	OK
DO
	GRAPHICS_TWO (FFNet, Eigen)
		FFNet_Eigen_drawIntersection (me, you, GRAPHICS, pcx, pcy, xmin, xmax, ymin, ymax);
	GRAPHICS_TWO_END
} 

/************************* FFNet && Categories **********************************/

DIRECT (NEW1_FFNet_Categories_to_ActivationList) {
	CONVERT_TWO (FFNet, Categories)
		autoActivationList result = FFNet_Categories_to_ActivationList (me, you);
	CONVERT_TWO_END (my name);
}

/************************* FFNet && Matrix **********************************/

FORM (NEW1_FFNet_weightsFromMatrix, U"Replace weights by values from Matrix", nullptr) {
	NATURALVAR (layer, U"Layer", U"1")
	OK
DO
	CONVERT_TWO (FFNet, Matrix)
		autoFFNet result = FFNet_weightsFromMatrix (me, you, layer);
	CONVERT_TWO_END (my name);
}

/************************* FFNet && PatternList **********************************/

FORM (GRAPHICS_FFNet_PatternList_drawActivation, U"Draw an activation", nullptr) {
	NATURALVAR (row, U"PatternList (row) number", U"1");
	OK
DO
	GRAPHICS_TWO (FFNet, PatternList)
		FFNet_PatternList_drawActivation (me, you, GRAPHICS, row);
	GRAPHICS_TWO_END
}

FORM (NEW1_FFNet_PatternList_to_Categories, U"FFNet & PatternList: To Categories", U"FFNet & PatternList: To Categories...") {
	RADIOVAR (categorizationgMethod, U"Categorization method", 1)
		RADIOBUTTON (U"Winner-takes-all")
		RADIOBUTTON (U"Stochastic")
	OK
DO
	GRAPHICS_TWO (FFNet, PatternList)
		autoCategories result = FFNet_PatternList_to_Categories (me, you, categorizationgMethod);
	CONVERT_TWO_END (my name, U"_", you -> name)
}

FORM (NEW1_FFNet_PatternList_to_ActivationList, U"To activations in layer", nullptr) {
	NATURALVAR (layer, U"Layer", U"1")
	OK
DO
	GRAPHICS_TWO (FFNet, PatternList)
		autoActivationList result = FFNet_PatternList_to_ActivationList (me, you, layer);
	CONVERT_TWO_END (my name, U"_", you -> name)
}

/*********** FFNet & PatternList & ActivationList **********************************/

FORM (REAL_FFNet_PatternList_ActivationList_getTotalCosts, U"FFNet & PatternList & ActivationList: Get total costs", U"FFNet & PatternList & ActivationList: Get total costs...") {
	RADIOVAR (costFunctionType, U"Cost function", 1)
		RADIOBUTTON (U"Minimum-squared-error")
		RADIOBUTTON (U"Minimum-cross-entropy")
	OK
DO
	NUMBER_THREE (FFNet, PatternList, ActivationList)
		double result = FFNet_PatternList_ActivationList_getCosts_total (me, you, him, costFunctionType);
	NUMBER_THREE_END (U"")
}

FORM (REAL_FFNet_PatternList_ActivationList_getAverageCosts, U"FFNet & PatternList & ActivationList: Get average costs", U"FFNet & PatternList & ActivationList: Get average costs...") {
	RADIOVAR (costFunctionType, U"Cost function", 1)
		RADIOBUTTON (U"Minimum-squared-error")
		RADIOBUTTON (U"Minimum-cross-entropy")
	OK
DO
	NUMBER_THREE (FFNet, PatternList, ActivationList)
		double result = FFNet_PatternList_ActivationList_getCosts_average (me, you, him, costFunctionType);
	NUMBER_THREE_END (U"")
}

FORM (MODIFY_FFNet_PatternList_ActivationList_learn, U"FFNet & PatternList & ActivationList: Learn", nullptr) {
	// NATURAL (U"Layer", U"1")
	NATURALVAR (maximumNumberOfEpochs, U"Maximum number of epochs", U"100")
	POSITIVEVAR (tolerance, U"Tolerance of minimizer", U"1e-7")
	RADIOVAR (costFunctionType, U"Cost function", 1)
		RADIOBUTTON (U"Minimum-squared-error")
		RADIOBUTTON (U"Minimum-cross-entropy")
	OK
DO
	MODIFY_FIRST_OF_THREE (FFNet, PatternList, ActivationList)
		FFNet_PatternList_ActivationList_learnSM (me, you, him, maximumNumberOfEpochs, tolerance, costFunctionType);
	MODIFY_FIRST_OF_THREE_END	
}
	

FORM (MODIFY_FFNet_PatternList_ActivationList_learnSlow, U"FFNet & PatternList & ActivationList: Learn slow", nullptr) {
	// NATURAL (U"Layer", U"1")
	NATURALVAR (maximumNumberOfEpochs, U"Maximum number of epochs", U"100")
	POSITIVEVAR (tolerance, U"Tolerance of minimizer", U"1e-7")
	LABEL (U"Specifics", U"Specific for this minimization")
	POSITIVEVAR (learningRate, U"Learning rate", U"0.1")
	REALVAR (momentum, U"Momentum", U"0.9")
	RADIOVAR (costFunctionType, U"Cost function", 1)
		RADIOBUTTON (U"Minimum-squared-error")
		RADIOBUTTON (U"Minimum-cross-entropy")
	OK
DO
	MODIFY_FIRST_OF_THREE (FFNet, PatternList, ActivationList)
		FFNet_PatternList_ActivationList_learnSD (me, you, him, maximumNumberOfEpochs, tolerance, learningRate, momentum, costFunctionType);
	MODIFY_FIRST_OF_THREE_END	
}

/*********** FFNet & PatternList & Categories **********************************/

FORM (REAL_FFNet_PatternList_Categories_getTotalCosts, U"FFNet & PatternList & Categories: Get total costs", U"FFNet & PatternList & Categories: Get total costs...") {
	RADIOVAR (costFunctionType, U"Cost function", 1)
		RADIOBUTTON (U"Minimum-squared-error")
		RADIOBUTTON (U"Minimum-cross-entropy")
	OK
DO
	NUMBER_THREE (FFNet, PatternList, Categories)
		double result = FFNet_PatternList_Categories_getCosts_total (me, you, him, costFunctionType);
	NUMBER_THREE_END (U" (total costs)")
}

FORM (REAL_FFNet_PatternList_Categories_getAverageCosts, U"FFNet & PatternList & Categories: Get average costs", U"FFNet & PatternList & Categories: Get average costs...") {
	RADIOVAR (costFunctionType, U"Cost function", 1)
		RADIOBUTTON (U"Minimum-squared-error")
		RADIOBUTTON (U"Minimum-cross-entropy")
	OK
DO
	NUMBER_THREE (FFNet, PatternList, Categories)
		double result = FFNet_PatternList_Categories_getCosts_average (me, you, him, costFunctionType);
	NUMBER_THREE_END (U" (average costs)")
}

FORM (MODIFY_FFNet_PatternList_Categories_learn, U"FFNet & PatternList & Categories: Learn", U"FFNet & PatternList & Categories: Learn...") {
	NATURALVAR (maximumNumberOfEpochs, U"Maximum number of epochs", U"100")
	POSITIVEVAR (tolerance, U"Tolerance of minimizer", U"1e-7")
	RADIOVAR (costFunctionType, U"Cost function", 1)
		RADIOBUTTON (U"Minimum-squared-error")
		RADIOBUTTON (U"Minimum-cross-entropy")
	OK
DO
	MODIFY_FIRST_OF_THREE (FFNet, PatternList, Categories)
		FFNet_PatternList_Categories_learnSM (me, you, him, maximumNumberOfEpochs, tolerance, costFunctionType);
	MODIFY_FIRST_OF_THREE_END
}

FORM (MODIFY_FFNet_PatternList_Categories_learnSlow, U"FFNet & PatternList & Categories: Learn slow", U"FFNet & PatternList & Categories: Learn slow...") {
	NATURALVAR (maximumNumberOfEpochs, U"Maximum number of epochs", U"100")
	POSITIVEVAR (tolerance, U"Tolerance of minimizer", U"1e-7")
	LABEL (U"Specifics", U"Specific for this minimization")
	POSITIVEVAR (learningRate, U"Learning rate", U"0.1")
	REALVAR (momentum, U"Momentum", U"0.9")
	RADIOVAR (costFunctionType, U"Cost function", 1)
		RADIOBUTTON (U"Minimum-squared-error")
		RADIOBUTTON (U"Minimum-cross-entropy")
	OK
DO
	MODIFY_FIRST_OF_THREE (FFNet, PatternList, Categories)
		FFNet_PatternList_Categories_learnSD (me, you, him, maximumNumberOfEpochs,tolerance, learningRate, momentum, costFunctionType);
	MODIFY_FIRST_OF_THREE_END
}

/*********** FFNet & PCA **********************************/

FORM (GRAPHICS_FFNet_PCA_drawDecisionPlaneInEigenspace, U"FFNet & PCA: Draw decision plane", nullptr) {
	NATURALVAR (unitNumber, U"Unit number", U"1")
	NATURALVAR (layer, U"Layer number", U"1")
	NATURALVAR (horizontalEigenvectorNumber, U"Horizontal eigenvector number", U"1")
	NATURALVAR (verticalEigenvectorNumber, U"Vertical eigenvector number", U"2")
	REALVAR (xmin, U"left Horizontal range", U"0.0")
	REALVAR (xmax, U"right Horizontal range", U"0.0")
	REALVAR (ymin, U"left Vertical range", U"0.0")
	REALVAR (ymax, U"right Vertical range", U"0.0")
	OK
DO
	GRAPHICS_TWO (FFNet, PCA)
		FFNet_Eigen_drawDecisionPlaneInEigenspace (me, you, GRAPHICS, unitNumber, layer, horizontalEigenvectorNumber, verticalEigenvectorNumber, xmin, xmax, ymin, ymax);
	GRAPHICS_TWO_END
}

/*********** PatternList & Categories **********************************/

FORM (NEW1_PatternList_Categories_to_FFNet, U"PatternList & Categories: To FFNet", U"PatternList & Categories: To FFNet...") {
	INTEGERVAR (numberOfUnitsInHiddenLayer1, U"Number of units in hidden layer 1", U"0")
	INTEGERVAR (numberOfUnitsInHiddenLayer2, U"Number of units in hidden layer 2", U"0")
	OK
DO
	CONVERT_TWO (PatternList, Categories)
		autoFFNet result = PatternList_Categories_to_FFNet (me, you, numberOfUnitsInHiddenLayer1, numberOfUnitsInHiddenLayer2);
	CONVERT_TWO_END (result -> name)
}

/*********** RBM & PatternList **********************************/

DIRECT (NEW1_RBM_PatternList_to_ActivationList) {
	CONVERT_TWO (RBM, PatternList)
		autoActivationList result = RBM_PatternList_to_ActivationList (me, you);
	CONVERT_TWO_END (my name, U"_", you -> name)
}

void praat_uvafon_FFNet_init () {
	Thing_recognizeClassesByName (classFFNet, NULL);

	praat_addMenuCommand (U"Objects", U"New", U"Feedforward neural networks", nullptr, 0, nullptr);
	praat_addMenuCommand (U"Objects", U"New", U"Create FFNet...", nullptr, 1, NEW1_FFNet_create);
	praat_addMenuCommand (U"Objects", U"New", U"Create iris example...", nullptr, 1, NEWMANY_FFNet_createIrisExample);
	praat_addMenuCommand (U"Objects", U"New", U"Advanced", nullptr, 1, nullptr);
	praat_addMenuCommand (U"Objects", U"New", U"Create FFNet (linear outputs)...", nullptr, 2, NEW1_FFNet_create_linearOutputs);
	praat_addMenuCommand (U"Objects", U"New", U"Create PatternList...", nullptr, 2, NEW1_PatternList_create);
	praat_addMenuCommand (U"Objects", U"New", U"Create Categories...", nullptr, 2, NEW1_Categories_create);

	praat_addAction1 (classFFNet, 0, U"FFNet help", nullptr, 0, HELP_FFNet_help);
	praat_addAction1 (classFFNet, 0, DRAW_BUTTON, nullptr, 0, nullptr);
	praat_addAction1 (classFFNet, 0, U"Draw topology", nullptr, 1, GRAPHICS_FFNet_drawTopology);
	praat_addAction1 (classFFNet, 0, U"Draw weights...", nullptr, 1, GRAPHICS_FFNet_drawWeights);
	praat_addAction1 (classFFNet, 0, U"Draw weights to layer...", nullptr, praat_DEPTH_1 | praat_HIDDEN, GRAPHICS_FFNet_drawWeightsToLayer);
	praat_addAction1 (classFFNet, 0, U"Draw cost history...", nullptr, 1, GRAPHICS_FFNet_drawCostHistory);
	praat_addAction1 (classFFNet, 0, QUERY_BUTTON, nullptr, 0, nullptr);
	praat_addAction1 (classFFNet, 0, U"Query structure", nullptr, 1, nullptr);
	praat_addAction1 (classFFNet, 1, U"Get number of layers", nullptr, 2, INTEGER_FFNet_getNumberOfLayers);
	praat_addAction1 (classFFNet, 1, U"Get number of outputs", nullptr, 2, INTEGER_FFNet_getNumberOfOutputs);
	praat_addAction1 (classFFNet, 1, U"Get number of hidden units...", nullptr, 2, INTEGER_FFNet_getNumberOfHiddenUnits);
	praat_addAction1 (classFFNet, 1, U"Get number of inputs", nullptr, 2, INTEGER_FFNet_getNumberOfInputs);
	praat_addAction1 (classFFNet, 1, U"Get number of hidden weights...", nullptr, 2, INTEGER_FFNet_getNumberOfHiddenWeights);
	praat_addAction1 (classFFNet, 1, U"Get number of output weights", nullptr, 2, INTEGER_FFNet_getNumberOfOutputWeights);
	praat_addAction1 (classFFNet, 1, U"Get category of output unit...", nullptr, 2, INFO_FFNet_getCategoryOfOutputUnit);
	praat_addAction1 (classFFNet, 1, U"Get output unit of category...", nullptr, 2, INTEGER_FFNet_getOutputUnitOfCategory);
	praat_addAction1 (classFFNet, 0, U"-- FFNet weights --", nullptr, 1, nullptr);
	praat_addAction1 (classFFNet, 1, U"Get bias...", nullptr, 1, REAL_FFNet_getBias);
	praat_addAction1 (classFFNet, 1, U"Get weight...", nullptr, 1, REAL_FFNet_getWeight);
	praat_addAction1 (classFFNet, 1, U"Get minimum", nullptr, 1, REAL_FFNet_getMinimum);
	praat_addAction1 (classFFNet, 0, MODIFY_BUTTON, nullptr, 0, nullptr);
	praat_addAction1 (classFFNet, 0, U"Set bias...", nullptr, 1, MODIFY_FFNet_setBias);
	praat_addAction1 (classFFNet, 0, U"Set weight...", nullptr, 1, MODIFY_FFNet_setWeight);
	praat_addAction1 (classFFNet, 0, U"Reset...", nullptr, 1, MODIFY_FFNet_reset);
	praat_addAction1 (classFFNet, 0, U"Select biases...", nullptr, 1, MODIFY_FFNet_selectBiasesInLayer);
	praat_addAction1 (classFFNet, 0, U"Select all weights", nullptr, 1, MODIFY_FFNet_selectAllWeights);
	praat_addAction1 (classFFNet, 0, EXTRACT_BUTTON, nullptr, 0, nullptr);
	praat_addAction1 (classFFNet, 0, U"Extract weights...", nullptr, 1, NEW_FFNet_extractWeights);
	praat_addAction1 (classFFNet, 0, U"Weights to Matrix...", nullptr, praat_DEPTH_1 | praat_HIDDEN, NEW_FFNet_weightsToMatrix);
	praat_addAction1 (classFFNet, 0, U"& PatternList: Classify?", nullptr, 0, HINT_hint_FFNet_and_PatternList_classify);
	praat_addAction1 (classFFNet, 0, U"& PatternList & Categories: Learn?", nullptr, 0, HINT_hint_FFNet_and_PatternList_and_Categories_learn);

	praat_addAction2 (classFFNet, 1, classActivationList, 1, U"Analyse", nullptr, 0, nullptr);
	praat_addAction2 (classFFNet, 1, classActivationList, 1, U"To Categories...", nullptr, 0, NEW1_FFNet_ActivationList_to_Categories);

	praat_addAction2 (classFFNet, 1, classEigen, 1, U"Draw", nullptr, 0, nullptr);
	praat_addAction2 (classFFNet, 1, classEigen, 1, U"Draw hyperplane intersections", nullptr, 0, GRAPHICS_FFNet_Eigen_drawIntersection);

	praat_addAction2 (classFFNet, 1, classCategories, 1, U"Analyse", nullptr, 0, nullptr);
	praat_addAction2 (classFFNet, 1, classCategories, 1, U"To ActivationList", nullptr, 0, NEW1_FFNet_Categories_to_ActivationList);

	praat_addAction2 (classFFNet, 1, classMatrix, 1, U"Modify", nullptr, 0, nullptr);
	praat_addAction2 (classFFNet, 1, classMatrix, 1, U"Weights from Matrix...", nullptr, 0, NEW1_FFNet_weightsFromMatrix);

	praat_addAction2 (classFFNet, 1, classPatternList, 1, U"Draw", nullptr, 0, nullptr);
	praat_addAction2 (classFFNet, 1, classPatternList, 1, U"Draw activation...", nullptr, 0, GRAPHICS_FFNet_PatternList_drawActivation);
	praat_addAction2 (classFFNet, 1, classPatternList, 1, U"Analyse", nullptr, 0, nullptr);
	praat_addAction2 (classFFNet, 1, classPatternList, 1, U"To Categories...", nullptr, 0, NEW1_FFNet_PatternList_to_Categories);
	praat_addAction2 (classFFNet, 1, classPatternList, 1, U"To ActivationList...", nullptr, 0, NEW1_FFNet_PatternList_to_ActivationList);

	praat_addAction3 (classFFNet, 1, classPatternList, 1, classActivationList, 1, U"Get total costs...", nullptr, 0, REAL_FFNet_PatternList_ActivationList_getTotalCosts);
	praat_addAction3 (classFFNet, 1, classPatternList, 1, classActivationList, 1, U"Get average costs...", nullptr, 0, REAL_FFNet_PatternList_ActivationList_getAverageCosts);
	praat_addAction3 (classFFNet, 1, classPatternList, 1, classActivationList, 1, U"Learn", nullptr, 0, nullptr);
	praat_addAction3 (classFFNet, 1, classPatternList, 1, classActivationList, 1, U"Learn...", nullptr, 0, MODIFY_FFNet_PatternList_ActivationList_learn);
	praat_addAction3 (classFFNet, 1, classPatternList, 1, classActivationList, 1, U"Learn slow...", nullptr, 0, MODIFY_FFNet_PatternList_ActivationList_learnSlow);

	praat_addAction3 (classFFNet, 1, classPatternList, 1, classCategories, 1, U"Get total costs...", nullptr, 0, REAL_FFNet_PatternList_Categories_getTotalCosts);
	praat_addAction3 (classFFNet, 1, classPatternList, 1, classCategories, 1, U"Get average costs...", nullptr, 0, REAL_FFNet_PatternList_Categories_getAverageCosts);
	praat_addAction3 (classFFNet, 1, classPatternList, 1, classCategories, 1, U"Learn", nullptr, 0, nullptr);
	praat_addAction3 (classFFNet, 1, classPatternList, 1, classCategories, 1, U"Learn...", nullptr, 0, MODIFY_FFNet_PatternList_Categories_learn);
	praat_addAction3 (classFFNet, 1, classPatternList, 1, classCategories, 1, U"Learn slow...", nullptr, 0, MODIFY_FFNet_PatternList_Categories_learnSlow);
	
	praat_addAction2 (classFFNet, 1, classPCA, 1, U"Draw decision plane...", nullptr, 0, GRAPHICS_FFNet_PCA_drawDecisionPlaneInEigenspace);
	
	praat_addAction2 (classPatternList, 1, classCategories, 1, U"To FFNet...", nullptr, 0, NEW1_PatternList_Categories_to_FFNet);
	
	praat_addAction2 (classRBM, 1, classPatternList, 1, U"To ActivationList", nullptr, 0, NEW1_RBM_PatternList_to_ActivationList);
		
	INCLUDE_MANPAGES (manual_FFNet_init)
}

/* End of file praat_FFnet_init.cpp */
