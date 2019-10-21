typedef struct SHT_info{
    int fileDesc; /* αναγνωριστικός αριθμός ανοίγματος αρχείου από το επίπεδο block */
    char* attrName; /* το όνομα του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */
    int attrLength; /* το μέγεθος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */
    long int numBuckets; /* το πλήθος των “κάδων” του αρχείου κατακερματισμού */
    char *fileName; /* όνομα αρχείου με το πρωτεύον ευρετήριο στο id */
}SHT_info;

typedef struct SecondaryRecord{
    Record record;
    int blockId; //Το block στο οποίο έγινε η εισαγωγή της εγγραφής στο πρωτεύον ευρετήριο.
}SecondaryRecord;

int secondary_hash_function(SHT_info header_info, Record record);

/*Η συνάρτηση SHT_CreateSecondaryIndex χρησιμοποιείται για τη δημιουργία και κατάλληλη
αρχικοποίηση ενός αρχείου δευτερεύοντος κατακερματισμού με όνομα sfileName για το αρχείο
πρωτεύοντος κατακερματισμού fileName. Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται 0, ενώ σε
διαφορετική περίπτωση -1.*/
int SHT_CreateSecondaryIndex( char *sfileName,  /* όνομα αρχείου */
                              char* attrName,   /* όνομα πεδίου-κλειδιού */
                              int attrLength,   /* μήκος πεδίου-κλειδιού */
                              int buckets,      /* αριθμός κάδων κατακερματισμού*/
                              char* fileName    /* όνομα αρχείου πρωτεύοντος ευρετηρίου*/
);

/*Η συνάρτηση SHT_OpenSecondaryIndex ανοίγει το αρχείο με όνομα sfileName και διαβάζει από το
πρώτο μπλοκ την πληροφορία που αφορά το δευτερευον ευρετηριο κατακερματισμού. Κατόπιν,
ενημερώνετε μια δομή όπου κρατάτε όσες πληροφορίες κρίνετε αναγκαίες για το αρχείο αυτό
προκειμένου να μπορείτε να επεξεργαστείτε στη συνέχεια τις εγγραφές του. Μια ενδεικτική δομή με τις
πληροφορίες που πρέπει να κρατάτε δίνεται στη παραπάνω. Αφού ενημερωθεί κατάλληλα η δομή πληροφοριών του
αρχείου, την επιστρέφετε. Σε περίπτωση που συμβεί οποιοδήποτε σφάλμα, επιστρέφεται τιμή NULL. Αν το αρχείο που
δόθηκε για άνοιγμα δεν αφορά αρχείο κατακερματισμού, τότε αυτό επίσης θεωρείται σφάλμα.*/
SHT_info* SHT_OpenSecondaryIndex( char *sfileName );

/*Η συνάρτηση SHT_CloseSecondaryIndex κλείνει το αρχείο που προσδιορίζεται μέσα στη δομή
header_info. Σε περίπτωση που εκτελεστεί επιτυχώς, επιστρέφεται 0, ενώ σε διαφορετική περίπτωση -1.
Η συνάρτηση είναι υπεύθυνη και για την αποδέσμευση της μνήμης που καταλαμβάνει η δομή που
περάστηκε ως παράμετρος, στην περίπτωση που το κλείσιμο πραγματοποιήθηκε επιτυχώς.*/
int SHT_CloseSecondaryIndex( SHT_info* header_info );

int Secondary_Insert_in_new_Block(int fileDesc, int blockId);

/*Η συνάρτηση HT_SecondaryInsertEntry χρησιμοποιείται για την εισαγωγή μίας εγγραφής στο αρχείο
κατακερματισμού. Οι πληροφορίες που αφορούν το αρχείο βρίσκονται στη δομή header_info, ενώ η
εγγραφή προς εισαγωγή προσδιορίζεται από τη δομή record, η οποία πρέπει να περιλαμβάνει το block
του πρωτεύοντος ευρετηρίου που υπάρχει η εγγραφή προς εισαγωγή. Σε περίπτωση που εκτελεστεί
επιτυχώς, επιστρέφεται 0, ενώ σε διαφορετική περίπτωση -1.*/
int SHT_SecondaryInsertEntry( SHT_info header_info, /* επικεφαλίδα του αρχείου*/
                              SecondaryRecord record /* δομή που προσδιορίζει την εγγραφή */
);

/*Η συνάρτηση αυτή χρησιμοποιείται για την εκτύπωση όλων των εγγραφών που υπάρχουν στο αρχείο
κατακερματισμού οι οποίες έχουν τιμή στο πεδίο-κλειδί του δευτερεύοντος ευρετηρίου ίση με value. Η
πρώτη δομή δίνει πληροφορία για το αρχείο κατακερματισμού, όπως αυτή έχει επιστραφεί από την
SHT_OpenIndex. Η δεύτερη δομή αντίστοιχα δίνει πληροφορία για το αρχείο κατακερματισμού, όπως
αυτή είχε επιστραφεί από την HT_OpenIndex. Για κάθε εγγραφή που υπάρχει στο αρχείο και έχει τιμή
στο πεδίο-κλειδί (όπως αυτό ορίζεται στην SHT_info) ίση με value, εκτυπώνονται τα περιεχόμενά της
(συμπεριλαμβανομένου και του πεδίου-κλειδιού). Να επιστρέφεται επίσης το πλήθος των blocks που
διαβάστηκαν μέχρι να βρεθούν όλες οι εγγραφές. Σε περίπτωση λάθους επιστρέφει -1.*/
int SHT_SecondaryGetAllEntries( SHT_info header_info_sht, /*επικεφαλίδα του αρχείου δευτερεύοντος ευρετηρίου*/
                                HT_info header_info_ht,/* επικεφαλίδα του αρχείου πρωτεύοντος ευρετηρίου*/
                                void *value /* τιμή του πεδίου-κλειδιού προς αναζήτηση */
);
