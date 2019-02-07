import numpy as np
from sklearn.linear_model import LogisticRegression
from sklearn import datasets
import csv

output_param = "/media/ajdar/HDD/FaraHDD/Projects/LogisticReg/params.csv"

outcome = "/media/ajdar/HDD/FaraHDD/Projects/LogisticReg/result.csv"

def main():

    # import some data to play with
    bcd = datasets.load_breast_cancer()
    X = bcd.data[:, :]  # we only take the first two features.
    Y = bcd.target

    logreg = LogisticRegression(C=1e5, solver='lbfgs', max_iter=15000)

    # Create an instance of Logistic Regression Classifier and fit the data.
    model = logreg.fit(X, Y)

    print("Model in-sample Accuracy: ", model.score(X,Y))


    params = model.coef_[0].tolist()
    bias = model.intercept_[0]

    # Write parameters in CSV file
    c = csv.writer(open(output_param, "w"))

    for par in params:
        c.writerow([par])
    c.writerow([bias])

    # Compute WX+b for all training examples
    i = 0
    test_case = []
    wxb = []

    while i < Y.size:
        test_case.append(bcd.data[i])
        i+=1

    for case in test_case:
        dot_prod = np.dot(model.coef_,case)+model.intercept_
        wxb.append(dot_prod[0])

    # Write results in CSV file
    c2 = csv.writer(open(outcome, "w"))
    for res in wxb:
        c2.writerow([res])

    '''
    #Conformance Check 
    test = bcd.data[13]
    paru = model.coef_
    stuff = np.dot(paru,test)
    bia = model.intercept_
    print(stuff)
    print(bias)
    print(stuff+bias)
    '''

if __name__=="__main__":
    main()
