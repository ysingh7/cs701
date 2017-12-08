
#include <stdio.h>
#include <stdlib.h>

struct cards_order_t{
   int val;			//value of the card
   struct cards_order_t* next;	//link to the card below this card; NULL if this is the last card
};

typedef struct cards_order_t cards_order;

//This function checks if the deck has been ordered correctly
//return: 1 if ordered;0 if not
int check_if_ordered(cards_order* head,int num_cards){
	
	int i;
	for(i=0;i<num_cards;i++){
		if((head->val!=(num_cards-i-1))||(head==NULL))
			return 0;
		//printf("%d\n",head->val);
		head=head->next;		
	}

	return 1;
}

void print_content(cards_order* head){

	while(head!=NULL){
		printf("%d\n",head->val);
		head=head->next;
	}
}

int main(int argc, char** argv){

	if(argc!=2){
		printf("usage:%s <number of cards in the deck>\n",argv[0]);
		return 1;
	}

	int num_cards=atoi(argv[1]);

	//Check if input is valid. Assumption - the deck can be greater than 52 cards, else
	//need to have a check below for <=52
	if(num_cards<1){
		printf("Enter a positive number of cards\n");
		return 1;
	}

	printf("Number of cards=%d\n",num_cards);

	int i;
	int num_rounds=0; //number of rounds the deck is shuffled 

	//pointers for the 2 decks that we will be operating on
	cards_order *curr1,*curr2,*head1,*head2,*tail1,*tail2;	

	//create the linked list to represent the initial order of cards
	head1=NULL;
	tail1=NULL;
	for(i=num_cards-1;i>-1;i--) {
		curr1 = (cards_order*)malloc(sizeof(cards_order));
		if(tail1==NULL)
			tail1=curr1;
		curr1->val = i;
		curr1->next  = head1;
		head1 = curr1;
	}


	int flag; // Flag to alternate between step 1 and step 2

	while(1){
	
	num_rounds++; //count the number of rounds performed

	head2=NULL;
	tail2=head1;
	curr2=NULL;
	
	flag=1;
	while(head1!=NULL){
		if(flag){
			head2=head1;		
			head1=head1->next;
			head2->next=curr2;
			curr2=head2;
			flag=0;
		}
		else{
			curr1=head1->next;
			tail1->next=head1;
			tail1=head1;
			tail1->next=NULL;
			head1=curr1;
			flag=1;
		}
	}
	tail1->next=head2;
	head2=tail1;

	//define the deck formed as Deck 1 and thus define head1 and tail1
	tail1=tail2;
	head1=head2;

	//print_content(head2);
	if(check_if_ordered(head2,num_cards-1)){
		printf("Ordered\n");
		printf("NUmber of rounds= %d\n",num_rounds);
		break;
	}

	}
	return 0;
}
