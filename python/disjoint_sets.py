class DisjointSets(object):
    def __init__(self):
        # maps elements -> sets
        self.elements = {}
        
    def createSet(self):
        return self.Set(self)

    def getAllSets(self):
        allSets = []
        for set in self.elements.values():
            if set.isRootSet():
                allSets.append(set)
        return allSets
        
    class Set(object):
        
        def __init__(self, disjointSets):
            self.disjointSets = disjointSets
            self.linkedSet = None

        def add(self, element):
            if self.disjointSets.elements.has_key(element):
                currentSetForElement = self.disjointSets.elements[element]
                currentSetForElement.getRootSet().linkedSet = self
            else:
                self.disjointSets.elements[element] = self


        def isMember(self, element):
            if self.disjointSets.elements.has_key(element):
                currentSetForElement = self.disjointSets.elements[element]
                return (currentSetForElement.getRootSet() == self.getRootSet())
            else:
                return False

        def members(self):
            members = []
            for element in self.disjointSets.elements.keys():
                if self.isMember(element):
                    members.append(element)
            return members

        def isRootSet(self):
            return (self.linkedSet == None)

        def getRootSet(self):
            if self.isRootSet():
                return self

            else:
                rootSet = self.linkedSet.getRootSet()
                # this is an optimization to keep
                # give sets as few links as possible to the root
                self.linkedSet = rootSet
                return rootSet
 
            
        def __repr__(self):
            return "Elements(" + repr(self.members()) + ")"
