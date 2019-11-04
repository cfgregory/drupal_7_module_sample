<?php
module_load_include('module', 'review_questions', 'review_questions');
function review_questions_admin($form, &$form_state) {
    
    
  
    
    $type_a = review_questions_content_type_select();
    $type_build_q = review_questions_content_type_review_questions();
    $type_has = array();
    foreach ($type_a as $key => $value) {   
        if(field_info_instance('node', 'field_review_questions', $key)){
            $type_has[] = $key;
            if(in_array($key, $type_build_q)){
                ${'ct_'.$key} = review_questions_review_questions_select($key);
            }
        }
        
    }
  $form['description'] = array(
    '#type' => 'item',
    '#title' => t('Select a Content Type'),
  );
  
  $form['content_types'] = array(
            '#id' =>  'content_types',
            '#name' => 'content_types',
            '#type' => 'checkboxes',
            '#title' => t('Content Types'),
            '#options' => $type_a,
            '#required' => TRUE,
            '#default_value' =>$type_has,
            
        );
   
   

    foreach ($type_build_q as $key => $value) {
        $form['ct_entry']['type'] = array(
            '#prefix' => '<div>Questions for content type of '.$value.' ',
            '#suffix' => '</div>',
        );
        
        
        for ($i=0; $i < count(${'ct_'.$key}); $i++) {
            $nid = ${'ct_'.$key}[$i]->entity_id;
            $node = node_load($nid);
            $fc_item = entity_load('field_collection_item', array($node->field_review_questions['und'][0]['value']));
            if(!empty($fc_item[1]->field_showquestion)){
            $name = 'answer_'.$key.'_'.$nid;
            $form['ct_entry'][$name] = array(
                '#id' =>  $name,
                '#name' => $name,
                '#type' => 'textarea',
                '#title' => t($fc_item[1]->field_question['und'][0]['value']),
                '#size' => 22,
                '#description' => t($fc_item[1]->field_question['und'][0]['value']),                
                '#required' => false,
            );
            }
             
            
    }
        }
        
        
   
        

    
   
   
   
   
   
   
   
   
   
   if (empty($form_state['email_num'])) {
        $form_state['email_num'] = 1;
    }
    $form['email_entry'] = array(
            '#prefix' => '<div id="entry-fieldset-wrapper">These addresses will be sent the results of the review questions form submissions',
            '#suffix' => '</div>',
        );
    for ($i = 0; $i < $form_state['email_num']; $i++) {
        $name = 'email_address_'.$i;
        $form['email_entry'][$name] = array(
            '#id' =>  $name,
            '#name' => $name,
            '#type' => 'textfield',
            '#title' => '',
            '#size' => 22,
            '#description' => t('Email address'),
            '#attributes' => array('multiple' => 'multiple'),
            '#required' => false,
        );
    }

    $form['email_entry']['add_more'] = array(
        '#type' => 'submit',
        '#value' => t('Add Another Email'),
        '#submit' => array('email_add_more_add_one'),
        '#ajax' => array(
            'callback' => 'email_add_more_callback',
            'wrapper' => 'entry-fieldset-wrapper',
        ),
    );

      
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit Answers',
    '#submit' => array('review_questions_admin_submit'),
    '#validate' => array('review_questions_admin_validate'),
  );
  return $form;
}


/**
 * Validation function for review_questions_admin().
 *
 * @ingroup review_questions_admin
 */
function review_questions_admin_validate($form, &$form_state) {
   for ($i=0; $i < count( $form_state['email_num']); $i++) { 
     $name = 'email_address_'.$i; 
     $email = $form_state['values'][$name];      
      if (!filter_var($email, FILTER_VALIDATE_EMAIL) && $email != '') {
          form_set_error('email_address_'.$i, t('Invalid email format for email.'.$email));
      }
 }  

}

/**
 * Submit function for review_questions_admin().
 *
 * Adds a submit handler/function to our form to send a successful
 * completion message to the screen.
 *
 * @ingroup review_questions_admin
 */
function review_questions_admin_submit($form, &$form_state) {
  global $user;  
 $date = new DateTime();
 $today = $date->getTimestamp();   
 $types_form = $form_state['values']['content_types'];
 $selections = array();
 
 foreach ($types_form as $key => $value) {         
     if($value != '0'){
       $selections[$key] = $key;
       review_questions_create_field_collection($key);  
     }
 }
 foreach ($selections as $key => $value) {
     for ($i=0; $i < count( $form_state['email_num']); $i++) { 
     $name = 'email_address_'.$i; 
     $email = $form_state['values'][$name];    
     $entry = array(
                'author_uid' => $user->uid,
                'date' => $today,
                'content_type' => $key,
                'email' => $email,
            );
            $return = review_questions_email_insert($entry);
       
 }
 }
 
 
  
}
function email_add_more_add_one($form, &$form_state) {

    if (!isset($form_state['email_num'])) {
        $form_state['email_num'] = 0;
        $form_state['email_num']++;
    }

    $form_state['email_num']++;
    $form_state['rebuild'] = TRUE;
}

function email_add_more_callback($form, $form_state) {
    return $form['email_entry'];
}


//data pulls 

function review_questions_content_type_select(){
    $query = db_select('node_type', 'nt');
    $query->fields('nt', array('type', 'name'));
    
    $result = $query->execute()->fetchAllKeyed(0,1);

   return $result;
    
    
}
function review_questions_review_questions_select($bundle){
    $query = db_select('field_data_field_review_questions', 'rq');
    $query->fields('rq', array('entity_type', 'bundle', 'entity_id'));
    $query->condition('rq.bundle', $bundle, '=');
    $result = $query->execute()->fetchAll();

   return $result;
    
    
}
function review_questions_content_type_review_questions(){
    
$query = db_select('field_data_field_review_questions', 'e');
  $query->AddExpression('distinct e.bundle', 'bundle');
 $result = $query->execute()->fetchAllKeyed(0,0);

   return $result;
    
    
}



    function review_questions_email_insert($entry) {
        $return_value = NULL;
        try {
            $return_value = db_insert('review_questions_email_address')
            ->fields($entry)
            ->execute();

            $lastId = Database::getConnection()->lastInsertId();
        }
        catch (Exception $e) {
            drupal_set_message(t('db_insert failed. Message = %message, query= %query',
            array("%message" => $e->getMessage(), "%query" => $e->query_string)), 'error');
        }

        return $return_value;
    }