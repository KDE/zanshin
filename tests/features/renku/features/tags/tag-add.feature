Feature: Tag creation
  As someone using notes
  I can create a tag
  In order to give them some semantic


  Scenario: New tags appear in the list
    Given I display the available pages
    When I add a "tag" named "Optional"
    And I list the items
    Then the list is:
       | display                           | icon                |
       | Inbox                             | mail-folder-inbox   |
       | Tags                              | folder              |
       | Tags / Optional                   | view-pim-tasks      |
       | Tags / Philosophy                 | view-pim-tasks      |
       | Tags / Physics                    | view-pim-tasks      |
