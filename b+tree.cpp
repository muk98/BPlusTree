#include "b+tree.h"

/*Following notations and part of procedures are taken from algorithms of Btree in CLRS*/  

BPlusTree::BPlusTree(int numIndexPointers,int numDataPointers){
	root = new DataNode(numDataPointers);
	this->numIndexPointers = numIndexPointers;
	this->numDataPointers = numDataPointers;
	depth = 0;
}

IndexNode* BPlusTree::toIndexNode(void *p){
	return static_cast<IndexNode*>(p);
}

DataNode* BPlusTree::toDataNode(void *p){
	return static_cast<DataNode*>(p);
}

DataNode* BPlusTree::search(void *node, int key, int depth){
	if(depth==0){
		DataNode *x = toDataNode(node);	
		for(int i = 0; i < x->getNumKeys(); i++)
			if(x->getKeyAtIndex(i) == key)
				return x;
		return NULL;	
	}
	else{
		IndexNode *x = toIndexNode(node);
		int i = 0;
		while(i < x->getNumKeys() && key >= x->getKeyAtIndex(i))
			i = i + 1;
		return search(x->getIndexPointerAt(i), key, depth-1);
	}
}




DataNode* BPlusTree::splitLeafChild(DataNode *x,int key,int &upKey){
	DataNode *z = new DataNode(numDataPointers);
	
	// inserting the key first
	int i = x->getNumKeys()-1;
	// cout<<"ssssffff"<<x->getKeyAtIndex(x->getNumKeys()-1)<<endl;
	while(i >= 0 && key < x->getKeyAtIndex(i))
		i = i - 1;
	i = i + 1;

	for(int j = x->getNumKeys()-1; j >= i; j--){
		x->setKeyAtIndex(x->getKeyAtIndex(j), j + 1);
	}

	x->setKeyAtIndex(key,i);
	x->setNumKeys(x->getNumKeys()+1);

	int mid = numDataPointers/2;
	
	z->setNumKeys(mid);
	
	for(int j = 0; j < mid; j++){
		z->setKeyAtIndex(x->getKeyAtIndex(j + mid+1), j);
		z->setDataPointerAt(x->getDataPointerAt(j + mid+1), j);
	}
	x->setNumKeys(mid+1);
	z->setDataNodeLeft(x);
	z->setDataNodeRight(x->getDataNodeRight());
	x->setDataNodeRight(z);
	upKey = z->getKeyAtIndex(0);
	// cout<<upKey;
	return z;
}


IndexNode* BPlusTree::splitNonLeafChild(IndexNode *x,void * newChildEntry,int &upKey){
	IndexNode *z = new IndexNode(numIndexPointers);
	int i = x->getNumKeys()-1;
	while(i >= 0 && upKey < x->getKeyAtIndex(i))
		i = i - 1;
	i = i + 1;


	for(int j = x->getNumKeys(); j > i; j--){
		x->setIndexPointerAt(x->getIndexPointerAt(j), j + 1);
	}

	x->setIndexPointerAt(newChildEntry,i+1);


	for(int j = x->getNumKeys()-1; j >= i; j--){
		x->setKeyAtIndex(x->getKeyAtIndex(j), j + 1);
	}

	//one extra entry , total even entries
	x->setKeyAtIndex(upKey,i);
	x->setNumKeys(x->getNumKeys()+1);

	int mid = (numIndexPointers+1)/2;

	upKey = x->getKeyAtIndex(mid);
	
	z->setNumKeys(mid-1);


	//moving the mid-1 values to z
	int j;
	for(j = 0; j < mid-1; j++){
		z->setKeyAtIndex(x->getKeyAtIndex(j + mid + 1), j);
		z->setIndexPointerAt(x->getIndexPointerAt(j + mid + 1), j);
	}
	z->setIndexPointerAt(x->getIndexPointerAt(j + mid + 1), j);
	
	x->setNumKeys(mid);
	return z;
}



void* BPlusTree::insert(BPlusTree* tree,void* node,int key,int depth,void *newChildEntry,int& upKey){
	// cout<<"yy";
	if(depth==0){

		DataNode* x = toDataNode(node);
		// cout<<"sss"<<x->getKeyAtIndex(0);
		if(x->getNumKeys() == numDataPointers-1){
			
			DataNode* newDataNode = splitLeafChild(x,key,upKey);
			
			if(x == tree->root){

				// if root is data node and needs to be splitted.
				cout<<"ROOT";
				IndexNode *new_root = new IndexNode(numIndexPointers);
				tree->root = new_root;
				new_root->setIndexPointerAt(x,0);
				
				new_root->setKeyAtIndex(upKey,0);
				new_root->setNumKeys(new_root->getNumKeys()+1);
				new_root->setIndexPointerAt(newDataNode,1);
				
				newChildEntry= NULL;
				upKey = -1;

				tree->depth = tree->depth + 1;
			}
			else{
				// if data node is not root send the key to index node
				newChildEntry = newDataNode;
				upKey = (toDataNode(newDataNode))->getKeyAtIndex(0);
				cout<<upKey<<endl;
				return newChildEntry;
			}
		}	
		else{

			// if data node is not full push the value at correct place.
			int i = x->getNumKeys()-1;
			cout<<x->getNumKeys()<<endl;
			while(i >= 0 && key < x->getKeyAtIndex(i)){
				x->setKeyAtIndex(x->getKeyAtIndex(i), i+1);
				i = i - 1;
			}
			x->setKeyAtIndex(key, i+1);
			x->setNumKeys(x->getNumKeys()+1);
		}
	}
	else{

		// if data node is not reached.
		IndexNode *x = toIndexNode(node);
		int i = x->getNumKeys()-1;
		while(i >= 0 && key < x->getKeyAtIndex(i))
			i = i - 1;
		i = i + 1;

		newChildEntry = insert(tree,x->getIndexPointerAt(i),key,depth-1,newChildEntry,upKey);


		if(newChildEntry==NULL) return newChildEntry;

		// if index is node is full
		if(x->getNumKeys() == numIndexPointers-1){
			newChildEntry = splitNonLeafChild(x,newChildEntry,upKey);
			if(x == tree->root){
			IndexNode *new_root = new IndexNode(numIndexPointers);
			tree->root = new_root;
			new_root->setIndexPointerAt(x,0);
			
			new_root->setKeyAtIndex(upKey,0);
			new_root->setNumKeys(new_root->getNumKeys()+1);
			new_root->setIndexPointerAt(newChildEntry,1);
			newChildEntry= NULL;
			upKey = -1;
			tree->depth = tree->depth + 1;
			}
		}	
		else{
			// if index node is not full accomodate the newchild entry
			// ith is the place at which recursion was called and hence the entry that is new child entry must accomodate there only
			for(int j = x->getNumKeys(); j > i; j--){
				x->setIndexPointerAt(x->getIndexPointerAt(j), j + 1);
			}

			x->setIndexPointerAt(newChildEntry, i+1);
			
			for(int j = x->getNumKeys()-1; j >= i; j--){
				x->setKeyAtIndex(x->getKeyAtIndex(j), j + 1);
			}
			x->setKeyAtIndex(upKey, i);
			x->setNumKeys(x->getNumKeys()+1);
			newChildEntry = NULL;
			upKey = -1;
		}

		
	}
	return newChildEntry;
}



void BPlusTree::LevelOrderTraversal(void *root, int depth){
	queue<pair<int,void *> > q;
	queue<int> level;
	int nodeID = 1;
	pair<int, void*> curNode;
	int curLevel=0,prevLevel=-1;
	depth = depth + 1;
	q.push({nodeID, root});
	level.push(0);
	cout<<"\n*** NodeID :: Values ***\n"<<endl;
	while(!q.empty()){
		curNode = q.front(); 
		q.pop();
		if(prevLevel!=level.front()){
			depth = depth-1;
			prevLevel = curLevel++;
		}
		level.pop();
		cout<<curNode.first<< " :: ";
		if(depth==0){
			DataNode *node = toDataNode(curNode.second);
			for(int i=0; i<node->getNumKeys(); i++){
				cout<<node->getKeyAtIndex(i)<<" ";
			}
			cout<<endl;
		}
		else{
			IndexNode *node = toIndexNode(curNode.second);
			for(int i=0; i <= node->getNumKeys(); i++){
				q.push({++nodeID, node->getIndexPointerAt(i)});
				level.push(curLevel);
			}
			for(int i=0; i<node->getNumKeys(); i++){
				cout<<node->getKeyAtIndex(i)<<" ";
			}
			cout<<endl;
		}
	}
	cout<<endl<<endl;
}


// void BPlusTree::LevelOrderTraversal(void* root,int depth)
// {
//     std::queue<void*> queue;
//     queue.push(root);
//     int childs=1;  
//     int travesedChild=0;
//     cout<<"************* Level Order Traversal ******************\n\n";
//     cout<<"Level      ::       Nodes   \n\n";
//     cout<<depth<<" :: ";
//     while (!queue.empty())
//     {
//     	if(depth>0){
//     		if(childs==travesedChild){
//     			IndexNode* current = toIndexNode(queue.front());
// 	    		childs = current->getNumKeys()+1;
// 	    		travesedChild=0;
// 	    		depth--;
// 	    		cout<<endl;
// 	    		cout<<depth<<" :: ";
//     		}
// 	    	else{
// 	    		travesedChild++;
// 	    		}
//    		}

//         if(depth!=0){
//         	IndexNode* current = toIndexNode(queue.front());
// 	        queue.pop();
// 	        int i;
// 	        for (int i = 0; i < current->getNumKeys() ; ++i)
// 	        {
// 	        	cout<<current->getKeyAtIndex(i)<<" ";
// 	        }
// 	        cout<<"||";
// 	        for (i = 0; i <current->getNumKeys()+1; i++)
// 	        {
// 	            queue.push(current->getIndexPointerAt(i));
// 	        }
// 	    }
// 	    else{
// 	    	DataNode* current = toDataNode(queue.front());
// 	        queue.pop();
// 	        int i;
// 	        for (int i = 0; i < current->getNumKeys() ; ++i)
// 	        {
// 	        	cout<<current->getKeyAtIndex(i)<<" ";
// 	        }
// 	        cout<<"||";
// 	    }
// 	}
// 	cout<<endl;
// }